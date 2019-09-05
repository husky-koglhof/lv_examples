/*
 WiFi.cpp - WiFi library for Mock

 Copyright (c) 2019 Christian Baumgartner. All rights reserved.
 This file is part of the pc_simulator for littlevgl environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "lv_wifi.h"

char _wifi [10][10] = {
		"One",
		"Two",
		"Third",
		"Four",
		"Five",
		"Six",
		"Seven",
		"Eight",
		"Nine",
		"Ten"
};

int _ssidEnc [10] = { 42, 37, 6, 49, 22, 96, 73, 80, 9, 34 };
wifi_auth_mode_t _encryption [10] = { WIFI_AUTH_OPEN, WIFI_AUTH_OPEN, WIFI_AUTH_WPA2_PSK, WIFI_AUTH_WPA2_PSK, WIFI_AUTH_WPA2_PSK,
						WIFI_AUTH_WPA2_PSK, WIFI_AUTH_WEP, WIFI_AUTH_WEP, WIFI_AUTH_OPEN, WIFI_AUTH_WPA2_PSK};

char * _ssid;
char * _pass;

/**
 * Return the current SSID associated with the network
 * @return SSID
 */
char * getSSID(int id)
{
    return _wifi[id];
}

/**
 * Return the current network RSSI.
 * @return  RSSI value
 */
int8_t getRSSI(int id)
{
    return _ssidEnc[id];
}

int scanComplete() {
	return 10;
}

wifi_auth_mode_t getEncryptionType(int id) {
	return _encryption[id];
}

char * getWiFiStatusText() {
	return "Connected";
}

wl_status_t getWiFiStatus() {
	LV_LOG_WARN("getWiFiStatus called");
	return WL_CONNECTED;
}

char * getLocalIP() {
	return "192.168.1.120";
}

char * getGatewayIP() {
	return "192.168.1.1";
}

char * getSubnetMask() {
	return "255.255.255.0";
}

char * getDNS() {
	return "8.8.8.8";
}

char * getHostName() {
	return "littlevgl_simulator";
}

void scanNetworks() {

}

void disconnectWiFi() {

}

void connectSoftAP(char * ssid, char * pass) {
	_ssid = ssid;
	_pass = pass;
}

void disconnectSoftAP() {

}
