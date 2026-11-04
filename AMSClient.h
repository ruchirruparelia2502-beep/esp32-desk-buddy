#pragma once

// ============================================================
//  AMSClient.h — Apple Media Service (AMS) BLE GATT client
//  Runs as an NimBLE peripheral + client on the same connection
// ============================================================

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "AppState.h"
#include "config.h"

class AMSClient : public NimBLEServerCallbacks,
                  public NimBLEClientCallbacks {
public:
  /**
   * Initialise NimBLE, set up server advertising, and register callbacks.
   * Call once from setup() — runs asynchronously from here.
   */
  static void begin();

  /**
   * Write a remote command byte to the AMS Remote Command characteristic.
   * Safe to call from any task (internally queued / mutex protected).
   */
  static void sendCommand(uint8_t cmd);

  // ---- NimBLEServerCallbacks ----
  void onConnect(NimBLEServer* pServer,
                 ble_gap_conn_desc* desc) override;
  void onDisconnect(NimBLEServer* pServer) override;

  // ---- NimBLEClientCallbacks ----
  void onConnect(NimBLEClient* pClient) override;
  void onDisconnect(NimBLEClient* pClient) override;

private:
  static AMSClient*          _instance;
  static NimBLEServer*       _pServer;
  static NimBLEClient*       _pClient;
  static NimBLECharacteristic* _pRemoteCmdChar;
  static NimBLECharacteristic* _pEntityUpdateChar;
  static bool                _clientReady;

  static void _startAdvertising();
  static void _connectAsClient(const NimBLEAddress& addr);
  static void _subscribeEntityUpdate();
  static void _entityUpdateNotifyCB(NimBLERemoteCharacteristic* pChar,
                                    uint8_t* pData, size_t length,
                                    bool isNotify);

  // Task that performs the client-side connection + subscription
  static void _clientTask(void* param);
};
