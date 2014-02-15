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
 *	@file MasterServer.h
 */

#ifndef __MASTERSERVER_H
#define __MASTERSERVER_H

#include <winsock2.h>
#include <fstream>
#include <sql.h>
#include <sqlext.h>
#include "Allegiance.h"
#include "DataBase.h"
#include "Fellowship.h"
#include "CorpseCleaner.h"
#include "cMonsterServer.h"
#include "SimpleAI.h"
#include "Status.h"

class cClient;
class cJobPool;

class cMasterServer
{
	friend class cClient;
	friend class cCommandParser;
	friend class cAltar;
	friend class cCovenant;
	friend class cDatabase;
	friend class cFellowship;
	friend class cHouse;
	friend class cMonsterServer;

public:
	static void				WriteToFile			( char *szMessage );
	static void				Load				( );
	static BOOL				Unload				( );

	static void				StartThread			( );
	static void				StopThread			( DWORD dwTimeOut );
	static DWORD WINAPI		ServerThread		( LPVOID lpVoid );
	
	static void				ParseCommand		( char *szCommand, WORD wSize, cClient *pcClient );
	//static void				ParseCommand		( char *szCommand, WORD wSize, cClient *pcClient, cNPC *npc );
	static void				LoadAvatar			( cClient *pcClient );
    static void				CreateNewAvatar		( cAvatar **ppcAvatar, DWORD dwGUID );
	static void				SendLoginData		( cClient *pcClient );
	static void				CreateInventory		( cClient *pcClient );
	static void				ServerMessage		( DWORD dwColor, cClient *pcClient, char *szMessage, ... );
	static void				SendTell			( char *szMessage, cClient *pcDestination, cClient *pcOrigin );

	static void CALLBACK	ScavengeIdleSockets	( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );
	static void CALLBACK	Status_Update		( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );
	static BOOL				DisconnectClient	( cClient *pcClient );
	static BOOL				Corpse				( cClient *pcClient );
	static BOOL				DisconnectAllClients( );
	static void				LoadMonsterModels	( );
	static void				LoadItemModels		( );
	static void				LoadWorldModels		( );
	static void				LoadMonsters		( );
	static void				LoadGroundSpawns	( );
	static void				LoadSpellModels		( );
	static void				LoadSpells			( );
	static void				LoadSpellComps		( );
	static void				LoadNPCs			( );
	static void				LoadAltars			( );
	static void				LoadCovenants		( );
	static void				LoadHousing			( );
	static void				LoadHooks			( );
	static void				LoadStorage			( );
	static void				LoadDoors			( );
	static void				LoadChests			( );
	static void				LoadLifestones		( );
	static void				LoadPortals			( );
	static void				LoadWorldObjects	( );
	static void				LoadWorldObjects2	( );
	static void				LoadNPCModels		( );
	//Load unique item data
	static void				LoadItem			( cObject *pcItem );

	static void				LoadSpawns			(WORD LB);

	static bool				SpawnMonster		( char* szMonster, cLocation pcLoc, DWORD Respawn = 0 );
	static bool				SpawnType			( char* szMonster, cLocation pcLoc, DWORD dwModelNumber, DWORD dwExp_Value, DWORD dwHealth, DWORD dwStamina, DWORD dwMana );
	static bool				SpawnSave			( char* szMonster, cLocation pcLoc, bool bFacing, bool bOverride, DWORD Respawn, DWORD Decay, DWORD Chase, DWORD Influence );
//	static bool				NPC_Save			( char* szName, cLocation pcLoc, bool bFacing );

	static BOOL				FindHeaderInFile	( FILE* fin, char* header );
	static void				FixSpaces			( char* str );
	static void				FixName				( char* str );
	
	static void				LoadStartingLocation( );
	static void				LoadTeleTownList	( );
	static void				LoadAllegiances		( );
	
	static void				ClearAllObjects		( );
	static BOOL				PKLite				( cClient *pcClient, bool bState );
	static CStatus			*cStatus;

	inline static void FormatIP( SOCKADDR_IN& saSockAddr, char *szStore )
	{
		wsprintf( szStore, "%u.%u.%u.%u", saSockAddr.sin_addr.S_un.S_un_b.s_b1, saSockAddr.sin_addr.S_un.S_un_b.s_b2, 
			saSockAddr.sin_addr.S_un.S_un_b.s_b3, saSockAddr.sin_addr.S_un.S_un_b.s_b4 );
	}

	inline static void FormatIP_Port( SOCKADDR_IN& saSockAddr, char *szStore )
	{
		wsprintf( szStore, "%u.%u.%u.%u:%u", saSockAddr.sin_addr.S_un.S_un_b.s_b1, saSockAddr.sin_addr.S_un.S_un_b.s_b2,
			saSockAddr.sin_addr.S_un.S_un_b.s_b3, saSockAddr.sin_addr.S_un.S_un_b.s_b4, saSockAddr.sin_port );
	}
	static CorpseCleaner *m_pcCorpse;
	static SimpleAI		 *m_pcSimpleAI;
	static cJobPool		 *m_pcJobPool;
	static char			m_szServerName[64];
	static DWORD		m_dwNumUsers;
	static DWORD		m_UserCount;
	static std::list < cEnchantment * >	m_lstEnchantments;
	static std::vector< cTeleTownList >	m_TeleTownList;
	static std::vector< cAllegiance * >	m_AllegianceList;
	static std::vector< cFellowship * >	m_FellowshipList;

	static inline DWORD	NewFellowID	( )	{ return ++m_dwFellowID; }
	

	//k109:  Added correct starting locs

	//K109: Begin changes:
	static cLocation	m_HoltburgWest;
	static cLocation	m_HoltburgSouth;
	static cLocation	m_ShoushiSE;
	static cLocation	m_ShoushiWest;
	static cLocation	m_YaraqNorth;
	static cLocation	m_YaraqEast;

	//These are starting Lifestones
	static cLocation	m_LSHoltburgWest;
	static cLocation	m_LSHoltburgSouth;
	static cLocation	m_LSShoushiSE;
	static cLocation	m_LSShoushiWest;
	static cLocation	m_LSYaraqNorth;
	static cLocation	m_LSYaraqEast;
	//End changes

protected:
	static DWORD		m_dwFellowID;

private:
	static HANDLE		m_hThread;
	static HANDLE		m_hExitEvent;
	static UINT_PTR		m_uipTimer;
	static UINT_PTR		m_uipStatusTimer;
	static DWORD		m_dwThreadID;
	
	static cLocation	m_StartingLoc;
	static cLocation    m_MarketLoc;
	
	static int			m_iLoadCount;
	static FILE*		pFile;
	
};

//Inline Functions
//================

#endif	// #ifndef __MASTERSERVER_H