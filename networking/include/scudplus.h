#pragma once

#include <stdint.h>
// addresses
#define gPauseGame  0x100016
#define gPressedBtn 0x100080
#define gCredits    0x1000f3
#define gCreditMode 0x100114
#define gLink 0x10011D
#define gCarNumber 0x10011C
//----------------------
#define gMainState 0x104006
#define gSubState  0x104005


#define msAtractMode 5 // 5
#define msMainMenu 17 // 11
#define msLoading 18 //  12
#define msRollingStart 19 // 13
#define msPreRacing    20
#define msRacing  21   // 15

#define gMainTimer 0x104010
#define gSubTimer  0x104008
#define gCourseLaps 0x104015
#define gCPUCounter 0x104018
#define gCarCount   0x10401C
#define gLocalPlayerCar 0x104F44
#define p1TMission 0x104f45 
#define gCourse 0x104F47 
//--------------------------------
#define gRealPlayers 0x10F024
#define gRPArrows 0x10F030
#define ENABLEARROWS 0xFF
//--------------------
#define bCanMove 0x00
#define bCarType 0x06
#define bCarOwner 0x07
#define bVelocimeter 0x18
#define bXPos 0x24
#define bYPos 0x1C
#define bZPos 0x20
#define bPitch 0x2A
#define bSpeed 0x34
#define bYaw 0x48
#define bCarNumber 0x98
#define bBrakeLight 0xBC
#define bSteer 0xC0 
#define bGasPedal 0xE4
#define bAIAccel 0x1e0

//----------------------------

#define cBus 0x0
#define cCat 0x1
#define cTank 0x2
#define cRocket 0x3
#define c911 0x4
#define cViper 0x5
#define cF40 0x6
#define cMcF1 0x7

//-----------------------------------

// Game Patches
#define SecretCarsAddr 0x2BB90

static uint32_t SecretCarsON = 0x60000000;
static uint32_t SecretCarsOFF = 0x4082001C;

#define SelCourseFixAddr 0x2ABE4
static uint32_t SelCourseFixON = 0x60000000;
static uint32_t SelCourseFixOFF = 0x409A000C;
#define DisableRetireAddr 0x0002883C
#define DisableRetireAddr2 0x00028714
#define DisableRetireAddr3 0x00028744
#define DisableRetireAddr4 0x00028748
static uint32_t DisableRetireON = 0x60000000;



const uint32_t pBase[8] = { 0x181200, 0x181500, 0x181800, 0x181B00, 0x181E00, 0x182100, 0x182400, 0x182700 };

const uint8_t CarValues[8] = {0x9, 0xB, 0x8, 0xA, 0xD, 0xF, 0xC, 0xE };

