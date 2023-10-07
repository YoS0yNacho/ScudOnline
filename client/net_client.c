/**********************************************************************************************
*
*   raylib_networking_smaple * a sample network game using raylib and enet
*
*   LICENSE: ZLIB
*
*   Copyright (c) 2021 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#include "scudplus.h"
#include "memory.h"
// implementation code for network game play interface


#include "net_client.h"

#define ENET_IMPLEMENTATION
#include "net_common.h"


// the player id of this client
int LocalPlayerId = -1;

// the enet address we are connected to
ENetAddress address = { 0 };

// the server object we are connecting to
ENetPeer* server = { 0 };

// the client peer we are using
ENetHost* client = { 0 };

// time data for the network tick so that we don't spam the server with one update every drawing frame

// how long in seconds since the last time we sent an update
double LastInputSend = -100;

// how long to wait between updates (20 update ticks a second)
double InputUpdateInterval = 1.0f / 20.0f;

double LastNow = 0;

int GameState = 0;

bool IsReady = FALSE;

//
// Data about players
typedef struct
{
	// true if the player is active and valid
	bool Active;

	// is the player ready for a race
	uint8_t State;

	uint8_t Car;

	uint8_t CarNumber;

	// the last known location of the player on the field
	Vector3 Position;

	float Pitch;
	float Roll;
	float Yaw;

	float Speed;

	uint8_t BrakeLight;

	// the direction they were going
	Vector3 Direction;

	// the time we got the last update
	double UpdateTime;

	//where we think this item is right now based on the movement vector
	Vector3 ExtrapolatedPosition;

	uint32_t Base;

}RemotePlayer;

// The list of all possible players
// this is the local simulation that represents the current game state
// it includes the current local player and the last known data from all remote players
// the client checks this every frame to see where everyone is on the field
RemotePlayer Players[MAX_PLAYERS] = { 0 };

// Connect to a server
void Connect(const char* serverAddress)
{
	// startup the network library
	enet_initialize();

	// create a client that we will use to connect to the server
	client = enet_host_create(NULL, 1, 1, 0, 0);

	// set the address and port we will connect to
	enet_address_set_host(&address, serverAddress);
	address.port = 4545;

	// start the connection process. Will be finished as part of our update
	server = enet_host_connect(client, &address, 1, 0);
}

// Utility functions to read data out of a packet

/// <summary>
/// Read a player position from the network packet
/// player positions are sent as two signed shorts and converted into floats for display
/// since this sample does everything in pixels, this is fine, but a more robust game would want to send floats
/// </summary>
/// <param name="packet"></param>
/// <param name="offset"></param>
/// <returns>A raylib Vector with the position in the data</returns>
Vector3 ReadPosition(ENetPacket* packet, size_t* offset)
{
	Vector3 pos = { 0 };
	pos.x = ReadFloat(packet, offset);
	pos.y = ReadFloat(packet, offset);
	pos.z = ReadFloat(packet, offset);

	return pos;
}

// functions to handle the commands that the server will send to the client
// these take the data from enet and read out various bits of data from it to do actions based on the command that was sent

// A new remote player was added to our local simulation
void HandleAddPlayer(ENetPacket* packet, size_t* offset)
{
	// find out who the server is talking about
	int remotePlayer = ReadByte(packet, offset);
	if (remotePlayer >= MAX_PLAYERS || remotePlayer == LocalPlayerId)
		return;

	Players[remotePlayer].Base = pBase[1];

	// set them as active and update the location
	Players[remotePlayer].Active = true;
	//Players[remotePlayer].Position = ReadPosition(packet, offset);
	//Players[remotePlayer].Pitch = ReadFloat(packet, offset);
	//Players[remotePlayer].Yaw = ReadFloat(packet, offset);
	//Players[remotePlayer].Speed = ReadFloat(packet, offset);
	//Players[remotePlayer].BrakeLight = ReadByte(packet, offset);
	//Players[remotePlayer].Car = ReadByte(packet, offset);
	//Players[remotePlayer].CarNumber = ReadByte(packet, offset);

	Players[remotePlayer].UpdateTime = LastNow;

	// In a more robust game, this message would have more info about the new player, such as what sprite or model to use, player name, or other data a client would need
	// this is where static data about the player would be sent, and any initial state needed to setup the local simulation
}

// A remote player has left the game and needs to be removed from the local simulation
void HandleRemovePlayer(ENetPacket* packet, size_t* offset)
{
	// find out who the server is talking about
	int remotePlayer = ReadByte(packet, offset);
	if (remotePlayer >= MAX_PLAYERS || remotePlayer == LocalPlayerId)
		return;

	// remove the player from the simulation. No other data is needed except the player id
	Players[remotePlayer].Active = false;
}

// The server has a new position for a player in our local simulation
void HandleUpdatePlayer(ENetPacket* packet, size_t* offset)
{
	// find out who the server is talking about
	int remotePlayer = ReadByte(packet, offset);
	if (remotePlayer >= MAX_PLAYERS || remotePlayer == LocalPlayerId || !Players[remotePlayer].Active)
		return;

	// update the last known position and movement
	Players[remotePlayer].Position = ReadPosition(packet, offset);
    Players[remotePlayer].Pitch = ReadFloat(packet, offset);
	Players[remotePlayer].Yaw = ReadFloat(packet, offset);
	Players[remotePlayer].Speed = ReadFloat(packet, offset);
	Players[remotePlayer].BrakeLight = ReadByte(packet, offset);
	Players[remotePlayer].Car = ReadByte(packet, offset);
	Players[remotePlayer].CarNumber = ReadByte(packet, offset);

	Players[remotePlayer].UpdateTime = LastNow;
	// in a more robust game this message would have a tick ID for what time this information was valid, and extra info about
	// what the input state was so the local simulation could do prediction and smooth out the motion
}

// process one frame of updates
void Update(double now, float deltaT)
{
	LastNow = now;
	// if we are not connected to anything yet, we can't do anything, so bail out early
	if (server == NULL)
		return;

	// Check if we have been accepted, and if so, check the clock to see if it is time for us to send the updated position for the local player
	// we do this so that we don't spam the server with updates 60 times a second and waste bandwidth
	// in a real game we'd send our normalized movement vector or input keys along with what the current tick index was
	// this way the server can know how long it's been since the last update and can do interpolation to know were we are between updates.
	if (LocalPlayerId >= 0 && now - LastInputSend > InputUpdateInterval)
	{
					// Pack up a buffer with the data we want to send
					uint8_t buffer[40] = { 0 }; // 9 bytes for a 1 byte command number and two bytes for each X and Y value
					buffer[0] = (uint8_t)UpdateInput;   // this tells the server what kind of data to expect in this packet
					*(float*)(buffer + 1) = (float)Players[LocalPlayerId].Position.x;
					*(float*)(buffer + 5) = (float)Players[LocalPlayerId].Position.y;
					*(float*)(buffer + 9) = (float)Players[LocalPlayerId].Position.z;
					*(float*)(buffer + 13) = (float)Players[LocalPlayerId].Pitch;
					*(float*)(buffer + 17) = (float)Players[LocalPlayerId].Yaw;
					*(float*)(buffer + 21) = (float)Players[LocalPlayerId].Speed;
					buffer[25] = (uint8_t)Players[LocalPlayerId].BrakeLight;
					buffer[26] = (uint8_t)Players[LocalPlayerId].Car;
					buffer[27] = (uint8_t)Players[LocalPlayerId].CarNumber;

					// copy this data into a packet provided by enet (TODO : add pack functions that write directly to the packet to avoid the copy)
					ENetPacket* packet = enet_packet_create(buffer, 40, ENET_PACKET_FLAG_RELIABLE);

					// send the packet to the server
					enet_peer_send(server, 0, packet);

					// NOTE enet_host_service will handle releasing send packets when the network system has finally sent them,
					// you don't have to destroy them

					// mark that now was the last time we sent an update
					LastInputSend = now;

	}

	// read one event from enet and process it
	ENetEvent Event = { 0 };

	// Check to see if we even have any events to do. Since this is a a client, we don't set a timeout so that the client can keep going if there are no events
	if (enet_host_service(client, &Event, 0) > 0)
	{
		// see what kind of event it is
		switch (Event.type)
		{
			// the server sent us some data, we should process it
			case ENET_EVENT_TYPE_RECEIVE:
			{
				// we know that all valid packets have a size >= 1, so if we get this, something is bad and we ignore it.
				if (Event.packet->dataLength < 1)
					break;

				// keep an offset of what data we have read so far
				size_t offset = 0;

				// read off the command that the server wants us to do
				NetworkCommands command = (NetworkCommands)ReadByte(Event.packet, &offset);

				// if the server has not accepted us yet, we are limited in what packets we can receive
				if (LocalPlayerId == -1)
				{
					if (command == AcceptPlayer)    // this is the only thing we can do in this state, so ignore anything else
					{
						// See who the server says we are
						LocalPlayerId = ReadByte(Event.packet, &offset);

						// Make sure that it makes sense
						if (LocalPlayerId < 0 || LocalPlayerId > MAX_PLAYERS)
						{
							LocalPlayerId = -1;
							break;
						}

						// Force the next frame to do an update by pretending it's been a very long time since our last update
						LastInputSend = -InputUpdateInterval;

						// We are active
						Players[LocalPlayerId].Active = true;

						// LocalPlayerBase
						Players[LocalPlayerId].Base = pBase[0];
						// Set our player at some location on the field.
						// optimally we would do a much more robust connection negotiation where we tell the server what our name is, what we look like
						// and then the server tells us where we are
						// But for this simple test, everyone starts at the same place on the field
						Players[LocalPlayerId].Position = (Vector3){ 100, 100, 100 };
					}
				}
				else // we have been accepted, so process play messages from the server
				{
					// see what the server wants us to do
					switch (command)
					{
						case AddPlayer:
							HandleAddPlayer(Event.packet, &offset);
							break;

						case RemovePlayer:
							HandleRemovePlayer(Event.packet, &offset);
							break;

						case UpdatePlayer:
							HandleUpdatePlayer(Event.packet, &offset);
							break;

						case MasterIsReady:
							GameState = 1;
							break;

						case RaceStart:
							GameState = 2;
							break;

					}
				}
				// tell enet that it can recycle the packet data
				enet_packet_destroy(Event.packet);
				break;
			}

			// we were disconnected, we have a sad
			case ENET_EVENT_TYPE_DISCONNECT:
				server = NULL;
				LocalPlayerId = -1;
				break;
		}
	}

	/// Update Memory Stuff
	uint8_t mode = MEM_ReadByte(gMainState);
	switch (mode)
	{
	    case msAtractMode: 
		{
			if (LocalPlayerId != 0 && GameState == 0) // Ignore if we are player 0
			{
				{
					MEM_WriteByte(gPauseGame, 0x0);
					// if player 0 is not ready block client
					//MEM_WriteByte(gCreditMode, 0x0C); // Force 2 coin
					//MEM_WriteByte(gCredits, 0);
				}
			}
			else
				MEM_WriteByte(gPauseGame, 0x1);
			break;
		}
		case msMainMenu:
		{
			PatchGame();
			Sleep(200);
			MEM_WriteByte(gLink, 0x01);
			MEM_WriteByte(gRPArrows, 0x3);
			break;
		}
		case msLoading:
		{
			if (!IsReady)
			{
				LocalPlayerIsReady();
			}

			if (GameState == 1 || GameState == 0)
			{
				MEM_WriteByte(gPauseGame, 0x0); // pause game and wait for others
			}
			else if (GameState == 2)
			{
				MEM_WriteByte(gPauseGame, 0x1); // continue
			}
			break;
		}
		
		case msRollingStart:
		case msPreRacing:
		case msRacing:
			MEM_WriteInt(gMainTimer, 3420);
			MEM_WriteByte(gRealPlayers, 0x2);
			MEM_WriteByte(gCarCount, 0x1);
			// update all the remote players with an interpolated position based on the last known good pos and how long it has been since an update
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (i == LocalPlayerId || !Players[i].Active)
					continue;

				double delta = LastNow - Players[i].UpdateTime;
				Players[i].ExtrapolatedPosition = Vector3Add(Players[i].Position, Vector3Scale(Players[i].Direction, (float)delta));
				MEM_WriteFloat((Players[i].Base + bXPos), Players[i].Position.x);
				MEM_WriteFloat((Players[i].Base + bYPos), Players[i].Position.y);
				MEM_WriteFloat((Players[i].Base + bZPos), Players[i].Position.z);
				MEM_WriteFloat((Players[i].Base + bPitch), Players[i].Pitch);
				MEM_WriteFloat((Players[i].Base + bYaw), Players[i].Yaw);
				MEM_WriteFloat((Players[i].Base + bSpeed), Players[i].Speed);
				MEM_WriteByte((Players[i].Base + bBrakeLight), Players[i].BrakeLight);
				MEM_WriteByte((Players[i].Base + bCarType), CarValues[Players[i].Car]);
				MEM_WriteByte((Players[i].Base + bCarNumber), Players[i].CarNumber);
				MEM_WriteInt((Players[i].Base + bAIAccel), 0xFFFFFFFF);   //disables car AI
			}
			break;
	}


}

// force a disconnect by shutting down enet
void Disconnect()
{
	// close our connection to the server
	if (server != NULL)
		enet_peer_disconnect(server, 0);

	// close our client
	if (client != NULL)
		enet_host_destroy(client);

	client = NULL;
	server = NULL;

	// clean up enet
	enet_deinitialize();
}

// true if we are connected and have been accepted
bool Connected()
{
	return server != NULL && LocalPlayerId >= 0;
}

int GetLocalPlayerId()
{
	return LocalPlayerId;
}

// add the input to our local position and make sure we are still inside the field
void UpdateLocalPlayer(float deltaT)
{
	// if we are not accepted, we can't update
	if (LocalPlayerId < 0)
		return;

    Players[LocalPlayerId].Car = MEM_ReadByte(gLocalPlayerCar);
	Players[LocalPlayerId].CarNumber = MEM_ReadByte(gCarNumber);

	Vector3 tempPos = Players[LocalPlayerId].Position;

	Players[LocalPlayerId].Position.x = MEM_ReadFloat(Players[LocalPlayerId].Base + bXPos);
	Players[LocalPlayerId].Position.y = MEM_ReadFloat(Players[LocalPlayerId].Base + bYPos);
	Players[LocalPlayerId].Position.z = MEM_ReadFloat(Players[LocalPlayerId].Base + bZPos);

	Players[LocalPlayerId].Speed = MEM_ReadFloat(Players[LocalPlayerId].Base + bSpeed);
	Players[LocalPlayerId].BrakeLight = MEM_ReadByte(Players[LocalPlayerId].Base + bBrakeLight);

	//Players[LocalPlayerId].Direction = Vector3Subtract(tempPos, Players[LocalPlayerId].Direction);

	Players[LocalPlayerId].Pitch = MEM_ReadFloat(Players[LocalPlayerId].Base + bPitch);
	//Players[LocalPlayerId].Roll = MEM_ReadFloat(Players[LocalPlayerId].Base);
	Players[LocalPlayerId].Yaw = MEM_ReadFloat(Players[LocalPlayerId].Base + bYaw);

	//-----------------------------------------------------------------------------------------------------

	// add the movement to our location
	//Players[LocalPlayerId].Position = Vector3Add(Players[LocalPlayerId].Position, Vector3Scale(*movementDelta, deltaT));

	// make sure we are in bounds.
	// In a real game both the client and the server would do this to help prevent cheaters
	/*
	if (Players[LocalPlayerId].Position.x < 0)
		Players[LocalPlayerId].Position.x = 0;

	if (Players[LocalPlayerId].Position.y < 0)
		Players[LocalPlayerId].Position.y = 0;

	if (Players[LocalPlayerId].Position.x > FieldSizeWidth - PlayerSize)
		Players[LocalPlayerId].Position.x = FieldSizeWidth - PlayerSize;

	if (Players[LocalPlayerId].Position.y > FieldSizeHeight - PlayerSize)
		Players[LocalPlayerId].Position.y = FieldSizeHeight - PlayerSize;
    */
	//Players[LocalPlayerId].Direction = *movementDelta;
}


void PatchGame()
{
	MEM_PatchWord(SelCourseFixAddr, SelCourseFixON);
	MEM_PatchWord(SecretCarsAddr, SecretCarsON);
	MEM_PatchWord(DisableRetireAddr, DisableRetireON);
	MEM_PatchWord(DisableRetireAddr2, DisableRetireON);
	MEM_PatchWord(DisableRetireAddr3, DisableRetireON);
	MEM_PatchWord(DisableRetireAddr4, DisableRetireON);
}

// get the info for a particular player

bool GetPlayerPos(int id, Vector3* pos)
{
	// make sure the player is valid and active
	if (id < 0 || id >= MAX_PLAYERS || !Players[id].Active)
		return false;

	// copy the location (real or extrapolated)
	if (id == LocalPlayerId)
		*pos = Players[id].Position;
	else
		*pos = Players[id].ExtrapolatedPosition;
	return true;
}


void LocalPlayerIsReady()
{
	// Pack up a buffer with the data we want to send
	IsReady = TRUE;
	uint8_t buffer[1] = { 0 }; // 9 bytes for a 1 byte command number and two bytes for each X and Y value
	buffer[0] = (uint8_t)PlayerIsReady;   // this tells the server what kind of data to expect in this packet

	// copy this data into a packet provided by enet (TODO : add pack functions that write directly to the packet to avoid the copy)
	ENetPacket* packet = enet_packet_create(buffer, 1, ENET_PACKET_FLAG_RELIABLE);

	// send the packet to the server
	enet_peer_send(server, 0, packet);
}
