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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "TelnetClient.h"

TelnetClient::TelnetClient(Client &client, const char* host, int port)
: port(port), host(host)
{
	telnetClient = &client;
}

TelnetClient::TelnetClient(Client &client, IPAddress ip, int port)
: port(port), hostIp(ip)
{
	telnetClient = &client;
	host = nullptr;
}

bool TelnetClient::connect() {
	if (telnetClient->connected())
		return true;

	if (host != nullptr) 
		return telnetClient->connect(host, port);

	return telnetClient->connect(hostIp, port);
}

bool TelnetClient::login(const char *username, const char *password)
{
	DEBUG_PRINT(F("login|connecting..."));
	if (connect()) {
		DEBUG_PRINT(F("login|connected!"));
		// here there will be the initial negotiation
		// listenUntil(':');
		listen();

		DEBUG_PRINT(F("login|sending username"));
		if (!send(username, false))
			return false;
		listenUntil(':');

		DEBUG_PRINT(F("login|sending password"));
		if (!send(password, false))
			return false;

		return waitPrompt();
	}
	else {
		DEBUG_PRINT(F("login|connection failed!"));
		return false;
	}
}

bool TelnetClient::sendCommand(const char *cmd)
{

	send(cmd);
	// negotiation until the server show the command prompt again
	if (strcmp(cmd, "exit") != 0) {
		return waitPrompt();
	}
	else {
		disconnect();
	}
}

void TelnetClient::disconnect()
{
	telnetClient->stop();
}

bool TelnetClient::send(const char *buf, bool waitEcho)
{
	uint8_t l_size = strnlen(buf, MAX_OUT_BUFFER_LENGTH);
	if (l_size == MAX_OUT_BUFFER_LENGTH) {
		DEBUG_PRINT(F("send|BAD INPUT"));
		return false;
	}

	char l_outBuffer[MAX_OUT_BUFFER_LENGTH];
	strncpy(l_outBuffer, buf, MAX_OUT_BUFFER_LENGTH);
	strncat(l_outBuffer, "\r\n", MAX_OUT_BUFFER_LENGTH);

	if (strlen(l_outBuffer) >= MAX_OUT_BUFFER_LENGTH) {
		DEBUG_PRINT(F("send|BAD INPUT"));
		return false;
	}

	l_size = strnlen(l_outBuffer, MAX_OUT_BUFFER_LENGTH);
	for (uint8_t i = 0; i < l_size; ++i) {
		if (l_outBuffer[i] > 0) {
			telnetClient->write(l_outBuffer[i]);
			print(l_outBuffer[i]);
			if (waitEcho) {
				while (telnetClient->available() == 0)
					delay(1);
				char inByte = telnetClient->read();
			}
		}
	}
	return true;
}

void TelnetClient::negotiate()
{

	byte verb, opt;
	byte outBuf[3] = {255, 0, 0};

	DEBUG_PRINT(F("negotiate|server:IAC"));
	verb = telnetClient->read();
	if (verb == -1)
		return;
	switch (verb) {
	case 255:
		//...no it isn't!
		DEBUG_PRINT(F("negotiate|server:IAC escape"));
		print(char(verb));
		break;
	case 251:
		// to a WILL statement...
		DEBUG_PRINT(F("negotiate|server:WILL"));
		opt = telnetClient->read();
		if (opt == -1)
			break;
		DEBUG_PRINT(F("negotiate|server opt: "));
		DEBUG_PRINT(opt);
		// always answer DO!
		outBuf[1] = 253;
		outBuf[2] = opt;
		telnetClient->write(outBuf, 3);
		telnetClient->flush();
		DEBUG_PRINT(F("negotiate|client:IAC"));
		DEBUG_PRINT(F("negotiate|client:DO"));
		break;
	case 252:
		DEBUG_PRINT(F("negotiate|server:WONT"));
		break;
	case 253:
		// to a DO request...
		DEBUG_PRINT(F("negotiate|server:DO"));
		opt = telnetClient->read();
		if (opt == -1)
			break;
		DEBUG_PRINT(F("negotiate|server opt: "));
		DEBUG_PRINT(opt);
		// alway answer WONT!
		outBuf[1] = 252;
		outBuf[2] = opt;
		telnetClient->write(outBuf, 3);
		telnetClient->flush();
		DEBUG_PRINT(F("negotiate|client:IAC"));
		DEBUG_PRINT(F("negotiate|client:WONT"));
		break;
	case 254:
		DEBUG_PRINT(F("negotiate|server:DONT"));
		break;
	}
}

void TelnetClient::listen()
{

	while (telnetClient->available() == 0)
		delay(1);

	byte inByte;
	unsigned long startMillis = millis();

	while (1) {
		if (telnetClient->available() > 0) {
			startMillis = millis();
			inByte = telnetClient->read();
			if (inByte <= 0) {
				DEBUG_PRINT(F("listen|what?"));
			}
			else if (inByte == 255)	{
				negotiate();
			}
			else {
				// is stuff to be displayed
				print(char(inByte));
			}
		}
		else if (millis() - startMillis > LISTEN_TOUT) {
			DEBUG_PRINT(F("listen|TIMEOUT!!!"));
			return;
		}
	}
}

bool TelnetClient::listenUntil(char c)
{

	byte inByte;
	// listen incoming bytes untile one char in the array arrive
	while (telnetClient->available() == 0)
		delay(1);

	do {
		if (telnetClient->available() > 0) {
			inByte = telnetClient->read();
			if (inByte <= 0) {
				DEBUG_PRINT(F("listen|what?"));
			}
			else if (inByte == 255)	{
				negotiate();
			}
			else {
				// is stuff to be displayed
				print(char(inByte));
			}
			if (char(inByte) == c) {
				DEBUG_PRINT(F("listenUntil|TERMINATOR RECEIVED"));
				return true;
			}
		}
	} while (1);
}

bool TelnetClient::waitPrompt()
{

	bool l_bLoop = false;
	unsigned long startMillis = millis();

	do {
		if (!listenUntil(m_promptChar))
			return false;
		char l_lastByte = telnetClient->read();
		do	{
			l_bLoop = telnetClient->available() > 0;
			if (l_bLoop) {
				DEBUG_PRINT(F("waitPrompt|FALSE PROMPT DETECTED"));
				print('\r');
				// print('\n');
				break;
			}
		} while (millis() - startMillis < PROMPT_REC_TOUT);

	} while (l_bLoop);

	// print('\n');
	// print('\r');
	DEBUG_PRINT(F("waitPrompt|END"));
	return true;
}

void TelnetClient::print(char c)
{
	// edit this function if you want a different output!
	Serial.print(c);
}


void TelnetClient::setPromptChar(char c)
{
	m_promptChar = c;
}
