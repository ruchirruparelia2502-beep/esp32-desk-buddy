// ============================================================
//  AMSClient.cpp — Apple Media Service (AMS) implementation
//
//  Flow:
//  1. ESP32 starts NimBLE server + advertises
//  2. iOS connects → onConnect(server)
//  3. ESP32 spawns _clientTask which connects back as GATT client
//     targeting the same peer address
//  4. Discovers AMS service on iOS, subscribes to EntityUpdate
//  5. Writes EntityUpdate subscription = Track entity
//  6. Receives metadata via notifications
// ============================================================

#include "AMSClient.h"
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>

// ---- Static member definitions ----
AMSClient*            AMSClient::_instance        = nullptr;
NimBLEServer*         AMSClient::_pServer         = nullptr;
NimBLEClient*         AMSClient::_pClient         = nullptr;
NimBLECharacteristic* AMSClient::_pRemoteCmdChar  = nullptr;
NimBLECharacteristic* AMSClient::_pEntityUpdateChar = nullptr;
bool                  AMSClient::_clientReady      = false;

// Peer address captured on server connect
static NimBLEAddress s_peerAddr("00:00:00:00:00:00");
static bool          s_peerAddrValid = false;

// ----------------------------------------------------------------
void AMSClient::begin() {
  _instance = new AMSClient();

  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  _pServer = NimBLEDevice::createServer();
  _pServer->setCallbacks(_instance);

  // We don't need to expose any services from our side —
  // iOS only needs us to be a connectable peripheral.
  // A generic Device Information service helps iOS trust the connection.
  NimBLEService* pDIS = _pServer->createService("180A");
  NimBLECharacteristic* pMN =
    pDIS->createCharacteristic("2A29",
                               NIMBLE_PROPERTY::READ);
  pMN->setValue("DeskBuddy");
  pDIS->start();

  _startAdvertising();
  Serial.println("[AMS] BLE server started, advertising...");
}

// ----------------------------------------------------------------
void AMSClient::_startAdvertising() {
  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->setName(BLE_DEVICE_NAME);
  pAdv->addServiceUUID("180A");        // DIS
  pAdv->addServiceUUID(AMS_SERVICE_UUID);
  pAdv->setScanResponse(true);
  pAdv->setMinPreferred(0x06);
  pAdv->setMaxPreferred(0x12);
  NimBLEDevice::startAdvertising();
}

// ----------------------------------------------------------------
//  NimBLEServerCallbacks
// ----------------------------------------------------------------
void AMSClient::onConnect(NimBLEServer* pServer,
                          ble_gap_conn_desc* desc) {
  Serial.printf("[AMS] iOS connected. Peer: %s\n",
                NimBLEAddress(desc->peer_id_addr).toString().c_str());

  s_peerAddr      = NimBLEAddress(desc->peer_id_addr);
  s_peerAddrValid = true;

  // Update global state
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    g_state.btSource = BTSource::IOS_BLE;
    xSemaphoreGive(g_state.mutex);
  }

  // Spawn client task to connect back as GATT client
  xTaskCreate(_clientTask, "ams_client", 8192, nullptr, 2, nullptr);
}

void AMSClient::onDisconnect(NimBLEServer* pServer) {
  Serial.println("[AMS] iOS disconnected. Restarting advertising...");
  s_peerAddrValid = false;
  _clientReady    = false;
  _pRemoteCmdChar = nullptr;

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    g_state.btSource       = BTSource::NONE;
    g_state.track          = TrackInfo{};
    g_state.currentScreen  = Screen::IDLE;
    xSemaphoreGive(g_state.mutex);
  }

  _startAdvertising();
}

// ----------------------------------------------------------------
//  NimBLEClientCallbacks
// ----------------------------------------------------------------
void AMSClient::onConnect(NimBLEClient* pClient) {
  Serial.println("[AMS] GATT client connected to iOS");
}

void AMSClient::onDisconnect(NimBLEClient* pClient) {
  Serial.println("[AMS] GATT client disconnected");
  _clientReady = false;
}

// ----------------------------------------------------------------
//  Client task — runs on separate FreeRTOS task
// ----------------------------------------------------------------
void AMSClient::_clientTask(void* param) {
  // Brief delay to let iOS finish its own connection setup
  vTaskDelay(pdMS_TO_TICKS(1500));

  if (!s_peerAddrValid) {
    Serial.println("[AMS] Client task: no peer address, aborting");
    vTaskDelete(nullptr);
    return;
  }

  Serial.printf("[AMS] Connecting as GATT client to %s\n",
                s_peerAddr.toString().c_str());

  _pClient = NimBLEDevice::createClient();
  _pClient->setClientCallbacks(_instance, false);
  _pClient->setConnectionParams(12, 12, 0, 51);
  _pClient->setConnectTimeout(5);

  if (!_pClient->connect(s_peerAddr)) {
    Serial.println("[AMS] GATT client connection failed");
    NimBLEDevice::deleteClient(_pClient);
    _pClient = nullptr;
    vTaskDelete(nullptr);
    return;
  }

  // Discover AMS service
  NimBLERemoteService* pAMSSvc =
    _pClient->getService(AMS_SERVICE_UUID);
  if (!pAMSSvc) {
    Serial.println("[AMS] AMS service not found on iOS. "
                   "Is Music app open?");
    _pClient->disconnect();
    NimBLEDevice::deleteClient(_pClient);
    _pClient = nullptr;
    vTaskDelete(nullptr);
    return;
  }
  Serial.println("[AMS] Found AMS service");

  // Get Remote Command characteristic
  NimBLERemoteCharacteristic* pRemoteCmd =
    pAMSSvc->getCharacteristic(AMS_REMOTE_CMD_UUID);
  if (!pRemoteCmd) {
    Serial.println("[AMS] RemoteCommand char not found");
    _pClient->disconnect();
    NimBLEDevice::deleteClient(_pClient);
    _pClient = nullptr;
    vTaskDelete(nullptr);
    return;
  }

  // Store pointer — wrap into our own holder via a small struct
  // We keep _pRemoteCmdChar as a raw pointer aliased here
  // (NimBLERemoteCharacteristic is compatible write target)
  // We'll store a reinterpret for sendCommand
  _pRemoteCmdChar = reinterpret_cast<NimBLECharacteristic*>(pRemoteCmd);

  // Get Entity Update characteristic
  NimBLERemoteCharacteristic* pEntityUpdate =
    pAMSSvc->getCharacteristic(AMS_ENTITY_UPDATE_UUID);
  if (!pEntityUpdate) {
    Serial.println("[AMS] EntityUpdate char not found");
    _pClient->disconnect();
    NimBLEDevice::deleteClient(_pClient);
    _pClient = nullptr;
    vTaskDelete(nullptr);
    return;
  }

  // Subscribe to Entity Update notifications
  if (!pEntityUpdate->subscribe(true, _entityUpdateNotifyCB)) {
    Serial.println("[AMS] Failed to subscribe to EntityUpdate");
    _pClient->disconnect();
    NimBLEDevice::deleteClient(_pClient);
    _pClient = nullptr;
    vTaskDelete(nullptr);
    return;
  }
  Serial.println("[AMS] Subscribed to EntityUpdate notifications");

  // Write subscription: Track entity, attrs Artist(0) + Album(1) + Title(2)
  // Format: [EntityID, Attr0, Attr1, Attr2]
  uint8_t subCmd[] = {
    AMS_ENTITY_TRACK,
    AMS_TRACK_ATTR_ARTIST,
    AMS_TRACK_ATTR_ALBUM,
    AMS_TRACK_ATTR_TITLE
  };
  pEntityUpdate->writeValue(subCmd, sizeof(subCmd), false);
  Serial.println("[AMS] Wrote EntityUpdate subscription (Track entity)");

  // Also subscribe to Player entity for play state
  uint8_t playerCmd[] = {
    AMS_ENTITY_PLAYER,
    AMS_PLAYER_ATTR_PLAY_INFO
  };
  pEntityUpdate->writeValue(playerCmd, sizeof(playerCmd), false);
  Serial.println("[AMS] Wrote EntityUpdate subscription (Player entity)");

  _clientReady = true;
  Serial.println("[AMS] AMS client ready");

  // Task must stay alive to keep the connection
  for (;;) {
    if (!_pClient || !_pClient->isConnected()) {
      Serial.println("[AMS] Client task: connection lost, exiting");
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  vTaskDelete(nullptr);
}

// ----------------------------------------------------------------
//  Notification handler — called from NimBLE task
// ----------------------------------------------------------------
void AMSClient::_entityUpdateNotifyCB(
    NimBLERemoteCharacteristic* pChar,
    uint8_t* pData, size_t length, bool isNotify) {

  if (length < 3) return;

  uint8_t entityID  = pData[0];
  uint8_t attrID    = pData[1];
  // uint8_t flags  = pData[2];  // truncation / other flags
  String  value     = "";
  if (length > 3) {
    value = String((char*)(pData + 3), length - 3);
  }

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

  if (entityID == AMS_ENTITY_TRACK) {
    switch (attrID) {
      case AMS_TRACK_ATTR_ARTIST: g_state.track.artist = value; break;
      case AMS_TRACK_ATTR_ALBUM:  g_state.track.album  = value; break;
      case AMS_TRACK_ATTR_TITLE:  g_state.track.title  = value; break;
      default: break;
    }
    g_state.lastMusicUpdateMs = millis();
  } else if (entityID == AMS_ENTITY_PLAYER) {
    if (attrID == AMS_PLAYER_ATTR_PLAY_INFO) {
      // "Playing", "Paused", "Stopped"
      g_state.track.isPlaying = (value == "Playing");
      if (value == "Playing" || value == "Paused") {
        g_state.lastMusicUpdateMs = millis();
      }
    }
  }

  xSemaphoreGive(g_state.mutex);

  Serial.printf("[AMS] Entity=%d Attr=%d Val='%s'\n",
                entityID, attrID, value.c_str());
}

// ----------------------------------------------------------------
//  Send a remote command to iOS
// ----------------------------------------------------------------
void AMSClient::sendCommand(uint8_t cmd) {
  if (!_clientReady || !_pClient || !_pClient->isConnected()) {
    Serial.println("[AMS] sendCommand: client not ready");
    return;
  }

  // Re-cast back to NimBLERemoteCharacteristic
  NimBLERemoteCharacteristic* pRC =
    reinterpret_cast<NimBLERemoteCharacteristic*>(_pRemoteCmdChar);
  if (!pRC) {
    Serial.println("[AMS] sendCommand: RemoteCommand char is null");
    return;
  }

  uint8_t buf[1] = { cmd };
  pRC->writeValue(buf, 1, false);
  Serial.printf("[AMS] Sent command 0x%02X\n", cmd);
}
