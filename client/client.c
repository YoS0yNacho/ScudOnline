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

//This is the client main for a simple networking game (max 8 players)
// it starts up a graphical client, connects to a server and runs the game, showing all players

// include raylib
#include "raylib.h"
#include <stdint.h>
#include "memory.h"

// include the networking interface
// we can't directly include networking in any file that uses raylib.h, so we abstract out the network gameplay to it's own file
#include "net_client.h"
#include "net_constants.h"

// a list of predefined colors based on the player lost
Color PlayerColors[MAX_PLAYERS] = { 0 };

void SetColors()
{
	PlayerColors[0] = WHITE;
	PlayerColors[1] = RED;
	PlayerColors[2] = GREEN;
	PlayerColors[3] = BLUE;
	PlayerColors[4] = PURPLE;
	PlayerColors[5] = GRAY;
	PlayerColors[6] = YELLOW;
	PlayerColors[7] = ORANGE;
}

// main game client
int main()
{
	if (!MEM_Init())
	{
		return 0;
	}

	MEM_UpdateEmuoffset();

	SetColors();

	// set up raylib
	InitWindow(FieldSizeWidth, FieldSizeHeight, "Client");
	SetTargetFPS(60);

	// start network connection
	bool connected = false;

	// if you want to connect to a server on another machine, change this, or ask the user for the server address
	
	Connect("127.0.0.1");
	// how fast in pixels per second we can move
	// NOTE : the server should send us all this data in a real game

	while (!WindowShouldClose())
	{
		// if we are connected, process our input for the network game play system
		if (Connected())
		
		{
			
			connected = true;
			
			UpdateLocalPlayer(GetFrameTime());  
		} 
		else if (connected)
		{
			// we got disconnected, try to connect again
			Connect("127.0.0.1");
			connected = false;
		}

		// let the network game system update
		// this will process any inbound events and update the local simulation
		Update(GetTime(), GetFrameTime());

		// draw our game screen
		BeginDrawing();
		ClearBackground(BLACK);

		//

		if (!Connected())
		{
			// we are not connected, so just wait until we are, this can take some time
			DrawText("Connecting", 0, 20, 20, RED);
		}
		else
		{
			// we are connected, and know what our player ID is, so show that to the player in our color
			DrawText(TextFormat("Player %d", GetLocalPlayerId()), 0, 20, 20, PlayerColors[GetLocalPlayerId()]);
			//DrawText(TextFormat("Laps %d", off), 0, 40, 20, BLUE);
			Vector3 pos; 
			Vector3 pos2;
			GetPlayerPos(GetLocalPlayerId(), &pos);

			for (int i = 0; i < 2; i++)
			{
				if(i == GetLocalPlayerId()){
					DrawText(TextFormat("X %f", pos.x), 0, 40, 10, WHITE);
					DrawText(TextFormat("Y %f", pos.y), 0, 50, 10, WHITE);
					DrawText(TextFormat("Z %f", pos.z), 0, 60, 10, WHITE);
				}
				else {
					GetPlayerPos(i, &pos2);
					DrawText(TextFormat("X %f", pos2.x), 0, 70, 10, SKYBLUE);
					DrawText(TextFormat("Y %f", pos2.y), 0, 80, 10, SKYBLUE);
					DrawText(TextFormat("Z %f", pos2.z), 0, 90, 10, SKYBLUE);
				}

			}
			// draw all active players, this includes our local player since the game system is maintaining the local simulation
			/*
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				
				Vector2 pos = { 0 };
				if (GetPlayerPos(i, &pos))
				{
					DrawRectangle((int)pos.x, (int)pos.y, PlayerSize, PlayerSize, PlayerColors[i]);
				}
	
			}
			*/
		}
		//DrawFPS(0, 0);
		EndDrawing();
	}
	// cleanup
	Disconnect();
	CloseWindow();

	return 0;
}