/*
* This file is part of UAS2.
*
* UAS2 is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* UAS2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with UASv1; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 *	@file Shared.h
 *	Defines shared structures and variables.
 */

#ifndef __SHARED_H
#define __SHARED_H
#pragma once

#include <winsock2.h>
#include <time.h>
#include <list>
#include <math.h>

#define WM_CLEAROBJECTS			WM_APP+1

#define MAX_PACKET_SIZE		0x1E4
#define MAX_FRAGMENT_SIZE	0x1D0
#define MAX_DATA_SIZE		0x1C0

#define MAX_CORPSES			1000
#define MAX_MONSTERS		5000
#define MAX_PETS			5000
#define DELAY				120
#define CORPSE_DELAY		60
#define ACTION_DELAY		1
#define TICKS_PER_SEC		10
#define MAX_LEVEL			126

//Allegiance
#define OLD_PASSUP			true	// XP share based upon number of fellows (vassal factor)
#define PASSED_XP_MULT		1		// XP passup multiplier
#define RECEIVED_XP_MULT	1		// XP reception multiplier
#define MAX_VASSALS			12		// maximum number of vassals
#define MAX_RANK			10		// maximum rank

//Fellowship
#define OLD_PASSTHROUGH		true	// XP share based upon number of fellows (fellow factor)
#define SHARED_XP_MULT		1		// XP sharing multiplier
#define OLD_FELLOW_RANGE	true	// maximum distance for 100% XP share
#define FELLOW_RANGE_MULT	1		// XP share distance multiplier
#define MAX_FELLOW_SIZE		9		// maximum number of fellows

#ifdef _DEBUG
#define SAFEDELETE( p )				\
	{								\
		try {						\
			delete (p);				\
			(p) = NULL;				\
		}							\
		catch ( ... ) {				\
			UpdateConsole( "\r\n Bad deletion attempt detected at %s %lu!!\r\n", __FILE__, __LINE__ );	\
		}							\
	}
#else	
#define SAFEDELETE( p )			\
	{							\
		delete (p);				\
		(p) = NULL;				\
	}
#endif

#ifdef _DEBUG		
#define SAFEDELETE_ARRAY( a )		\
	{								\
		try {						\
			delete[] (a);			\
			(a) = NULL;				\
		}							\
		catch ( ... ) {				\
			UpdateConsole( "\r\n Bad array deletion attempt detected at %s %lu!!\r\n", __FILE__, __LINE__ );	\
		}							\
	}
#else
#define SAFEDELETE_ARRAY( a )	\
	{							\
		delete[] (a);			\
		(a) = NULL;				\
	}
#endif


#define		CHECKRETURN(ExitOnError, hType, hName, ReturnOnError)					\
			if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA))		\
			{																		\
				UpdateConsole( " <SQL> %s: Error on line %lu: ", __FILE__, __LINE__ );\
				cDatabase::GetError(hType, &hName);									\
				if (ExitOnError)													\
				{																	\
					UpdateConsole(" <SQL> Function has been aborted.\r\n");			\
					/*return ReturnOnError;	*/										\
				}																	\
				else																\
					UpdateConsole(" <SQL> Function has been continued normally.\r\n");	\
			}

class cClient;
typedef std::list< cClient * >::iterator iterClient_lst;

class cObject;
typedef std::list< cObject * >::iterator iterObject_lst;

class cModels;
typedef std::list< cModels * >::iterator iterModel_lst;

class cMagicModels;
typedef std::list< cMagicModels * >::iterator iterMagicModel_lst;

class cEnchantment;
typedef std::list< cEnchantment * >::iterator iterEnchantment_lst;

class cNPC;
typedef std::list< cNPC * >::iterator iterNPCs_lst;

#pragma pack( push, 1 )
typedef struct
{
	DWORD	m_dwSequence;	//! 0x00 - 0x03
	DWORD	m_dwFlags;		//! 0x04 - 0x07
	DWORD	m_dwCRC;		//! 0x08 - 0x0B
	WORD	m_wLogicalID;	//! 0x0C - 0x0D
	WORD	m_wTime;		//! 0x0E - 0x0F
	WORD	m_wTotalSize;	//! 0x10 - 0x11
	WORD	m_wTable;		//! 0x12 - 0x13
} cTransportHeader;

typedef struct
{
	DWORD	m_dwSequence;		//! Sequence/ID - Identifier used to match fragmented messages together.
	DWORD	m_dwObjectID;		//! Flags - Bitmask with an unknown purpose. Always 0x80000000?
	WORD	m_wFragmentCount;	//! Count - The number of fragments required to complete this message.
	WORD	m_wFragmentLength;	//! Size - Number of bytes in this fragment, including the fragment header.
	WORD	m_wFragmentIndex;	//! Index - Fragment index. Used to order the message data from the fragment.
	WORD	m_wFragmentGroup;	//! Group - Message group identifier. Message groups signify the message types.
} cFragmentHeader;
#pragma pack( pop )

#pragma pack( push, 1 )
typedef struct cLocation
{
	//std::string Name;
	DWORD	m_dwLandBlock;	//! landblock location
	float	m_flX;			//! cartesian position (relative to landblock)
	float	m_flY;			//! cartesian position (relative to landblock)
	float	m_flZ;			//! cartesian position (relative to landblock)
	float	m_flA;			//! quaternion orientation
	float	m_flB;			//! quaternion orientation
	float	m_flC;			//! quaternion orientation
	float	m_flW;			//! quaternion orientation

	cLocation( )
	{
		m_dwLandBlock	= 0x00;
		m_flA			= 0.624711f;
		m_flB			= 0.000000f;
		m_flC			= 0.000000f;
		m_flW			= 0.000000f;
	}
} cLocation;
#pragma pack( pop )

#pragma pack( push, 1 )
typedef struct cVelocity
{
	float m_dx, m_dy, m_dz;

	cVelocity( )
	{ 
		m_dx		= 0.0f;
		m_dy		= 0.0f;
		m_dz		= 0.0f;
	}
} cVelocity;
#pragma pack( pop )

typedef struct{
	BYTE bLow;
	BYTE bOO;
	BYTE bLo;
	BYTE bHi;		
} lb;

#pragma pack( push,1)
typedef struct sPaletteChange
{
	WORD m_wNewPalette;
	BYTE m_ucOffset;
	BYTE m_ucLength;
	sPaletteChange()
	{
		m_wNewPalette	= 0x0;	//! Palette (0x04000000 file)
		m_ucOffset		= 0x0;	//! Offset from beginning of palette
		m_ucLength		= 0x0;	//! Number of palette entries to copy
	}
} sPaletteChange;
#pragma pack( pop)

#pragma pack( push,1)
typedef struct sTextureChange
{
	BYTE m_bModelIndex;
	WORD m_wOldTexture;
	WORD m_wNewTexture;
	sTextureChange()
	{
		m_bModelIndex	= 0x0;	//! Index of body part affected
		m_wOldTexture	= 0x0;	//! Texture mask (0x05000000 file)
		m_wNewTexture	= 0x0;	//! Texture (0x05000000 file)
	}
} sTextureChange;
#pragma pack( pop)

#pragma pack( push,1)
typedef struct sModelChange
{
	BYTE m_bModelIndex;
	WORD m_wNewModel;
	sModelChange()
	{
		m_bModelIndex	= 0x0;	//! Index of body part affected
		m_wNewModel		= 0x0;	//! Model (0x02000000 file)
	}
} sModelChange;
#pragma pack( pop)

#pragma pack( push, 1 )
typedef struct cMonStats
{
	DWORD	m_dwLevel; 
	DWORD	m_dwStr; 
	DWORD	m_dwEnd; 
	DWORD	m_dwQuick; 
	DWORD	m_dwCoord; 
	DWORD	m_dwFocus;
	DWORD	m_dwSelf;
	DWORD	m_dwSpecies;


	cMonStats( )
	{
		m_dwLevel	= 0x1; 
		m_dwStr		= 0x0; 
		m_dwEnd		= 0x0; 
		m_dwQuick	= 0x0; 
		m_dwCoord	= 0x0; 
		m_dwFocus	= 0x0;
		m_dwSelf	= 0x0;
		m_dwSpecies	= 0x0;
	}
} cMonStats;
#pragma pack( pop )

enum eColors
{
	ColorGreen		= 1,
	ColorWhite		= 2,
	ColorYellow		= 3,
	ColorBrown		= 4,
	ColorMagenta	= 5,
	ColorRed		= 6,
	ColorGreen2		= 7,
	ColorPink		= 8,
	ColorLightPink	= 9,
	ColorYellow2	= 10,
	ColorBrown2		= 11,
	ColorGrey		= 12,
	ColorCyan		= 13,
	ColorAquamarine	= 14,
	ColorRed2		= 15,
	ColorGreen3		= 16,
	ColorBlue		= 17,
	ColorGreen4		= 18
};

struct cAvatarList
{
	DWORD		m_dwGUID;		//! Avatar GUID
	DWORD		m_dwOwnerID;	//! Account ID
	std::string	m_strName;		//! Avatar Name
};

enum eAccessLevels
{
	eDeveloper	= 0,
	eAdmin		= 1,
	eSentinel	= 2,
	eAdvocate	= 3,
	eStaff		= 4,
	eUeber		= 5,
	eVIP		= 6,
	eNormal		= 7
};

struct ConfirmPanel
{
	DWORD		m_dwSequence;
	DWORD		m_dwType;
	std::string	m_szText;

	DWORD		m_dwSenderGUID;
	DWORD		m_dwReceiptGUID;
};

struct cTeleTownList
{
	std::string	m_teleString;	//! String representing the town name
	DWORD		m_dwLandblock;
	FLOAT		m_flPosX;
	FLOAT		m_flPosY;
	FLOAT		m_flPosZ;
	FLOAT		m_flOrientW;
	FLOAT		m_flOrientX;
	FLOAT		m_flOrientY;
	FLOAT		m_flOrientZ;
};

struct cDungeonList
{
	INT			m_intID;
	std::string	m_strName;
	DWORD		m_dwLandblock;
	FLOAT		m_flLocX;
	FLOAT		m_flLocY;
	FLOAT		m_flLocZ;
	FLOAT		m_flHeading;
};

typedef struct cAnimates
{
	WORD	m_wStance[10];
	WORD	m_wAttack[10]; 
	WORD	m_wIdle[10]; 
	WORD	m_wReact[10];
	WORD	m_wDeath;
	WORD	m_wStand;
	cAnimates( )
	{
		for (int i = 0; i < 10; i++)
		{
			m_wStance[i] = 0x0000;
			m_wAttack[i] = 0x0000;
			m_wIdle[i]	 = 0x0000;
			m_wReact[i]	 = 0x0000;
		}
		m_wDeath	=	0x0011;
		m_wStand	=	0x0001;
	}
} cAnimates;

////////////////////////////////////////////////////////
// Define the Various Spell Structures
////////////////////////////////////////////////////////
typedef struct cSpellBook
{
	DWORD	dwSpell_ID;		//! Max 100 spells in the spellbook
	float	flCharge;		//! spell charge value
	DWORD	dwUnknownA;		//! 0 - so far
	DWORD	dwUnknownB;		//! 0 - so far
	WORD	wSpellType;		//! 0 - War, 1 - Life, 2 - Creature, 4 - Item
	
	cSpellBook()
	{
		dwSpell_ID = 0;
		flCharge = 1.0f;
		dwUnknownA = 0;
		dwUnknownB = 0;
		wSpellType = 0;
	}
} cSpellBook;

typedef struct cSpellTab
{
	DWORD	dwTabCount;
	DWORD	dwSpell_ID[40]; //! max 40 spells per tab
	
	cSpellTab()
	{
		dwTabCount = 0;
		for(int i = 0;i < 41;i++)
		{
			dwSpell_ID[i] = 0;
		}
	}
} cSpellTab;


// cs_CastMagic (Packet F7B1:004A - Group ??)
// - Cast Magic Spell
typedef struct cs_CastMagic
{
	DWORD	dwWorldEvent;
	DWORD	dwSequence;
	DWORD	dwGameEvent; //! should be 0x004A
	float	flHeading;
	DWORD	dwSpellID;
} cs_CastMagic;

////////////////////////////////////////////////////////
// Define the Inventory Sturcture
////////////////////////////////////////////////////////

typedef struct vInventory
{
	DWORD	dwObjectGUID;		//! Item GUID
	DWORD	dwIsContainer;		//! Is Container (0 = No, 1 = Yes)
	DWORD	fEquipped;			//! Is Equipped (0 = No, 1 = Yes in hand, 2 = Yes as clothing)
	DWORD	dwItemModelNumber;	//! Item's model number
	std::string	strName;		//! Item Name

	vInventory()
	{
		dwObjectGUID = 0;
		dwIsContainer = 0;
		fEquipped = 0;
		dwItemModelNumber = 0;
		strName = " ";
	}
} vInventory;

#pragma pack( push,1)
typedef struct sMonsters
{
	DWORD		m_dwGUID;			//! Monster GUID
	DWORD		m_dwLastAttacker;	//! Last attacker's GUID
	time_t		m_TimeNextAction;	//! Time to next action
	DWORD		m_dwEvent;

	sMonsters()
	{
		m_dwGUID			= 0x0L;
		m_dwLastAttacker	= 0x0L;
		time(&m_TimeNextAction + 30);
		m_dwEvent			= 0x0L;
	}
} sMonsters;
#pragma pack( pop)

#define Race 3
#define Gender 2
#define TexType 3
#define TexIndex 45

extern short	g_nCharPort;
extern short	g_nWorldPort;
extern HWND		g_hWndMain;
extern char		g_szLocalIP[16];
extern WORD		g_wAvatarTexturesList[Race][Gender][TexType][TexIndex];
extern WORD		g_wAvatarTexturesBaldList[Race][Gender][TexIndex];

void UpdateConsole( const char *szBuff );
long UpdateConsole( char *szBuff, long nErr );
void UpdateConsole( char *szMessage, ... );

#endif	// #ifndef __SHARED_H