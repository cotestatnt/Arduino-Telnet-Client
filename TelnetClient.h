/*
 * Copyright 2017 Alessio Villa
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have negotiationd a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
#ifndef TELNETCLIENT_H
#define TELNETCLIENT_H

#include "Arduino.h"
#include "Client.h"

#define TNDBG 0

#if TNDBG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

#define ARRAYSIZE(arr) (sizeof(arr) / sizeof(arr[0]))
const uint8_t NEGOTIATION_DELAY = 100;

////////////////CONFIGURATION////////////////////////////////////////////////////////////////////////////////////////////////////
//how long the command sent may be long
const uint8_t MAX_OUT_BUFFER_LENGTH = 150;
//how long you'll wait for an expected answer from the server
const unsigned int LISTEN_TOUT = 5000;
//how long, after a "prompt char" is received you can confirm it's the real prompt and not just part of the server's answer
const uint16_t PROMPT_REC_TOUT = 300;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TelnetClient{

public:

    TelnetClient(Client &client, const char* host, int port = 23);
	TelnetClient(Client &client, IPAddress ip, int port = 23);
	~TelnetClient(){};

	bool connect();
    bool login(const char* username, const char* password);
	bool sendCommand(const char* cmd);
    void disconnect();
    void setPromptChar(char c);
	void listen();

private:

	Client* telnetClient;
	char m_promptChar = '>';
	int  port;
	const char* host;
	IPAddress   hostIp;

	bool send(const char* buf, bool waitEcho = true);
	void negotiate();
	bool listenUntil(char c);
	bool waitPrompt();
	void print(char c);
};

#endif
