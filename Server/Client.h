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
 *	@file Client.h
 */

#ifndef __CLIENT_H
#define __CLIENT_H

#include <winsock2.h>

#include "Avatar.h"
#include "CannedPackets.h"
#include "CharacterServer.h"
#include "PacketPipe.h"
#include "WorldServer.h"
#include "Job.h"

typedef enum { NO_SERVER = 0, CHAR_SERVER, WORLD_SERVER } SERVER;

class cRecvPacket;
class cMessage;

class cClient
{
	friend class cMasterServer;
	friend class cWorldManager;
	friend class cCommandParser;
	friend class cAllegiance;
	friend class cFellowship;
	friend class cNPC;
	friend class cAltar;
	friend class cDoor;
	friend class cChest;
	friend class cLifestone;
	friend class cWeapon;
	friend class cAbiotic;
	friend class cPortal;
	friend class cArmor;
	friend class cCovenant;
	friend class cHouse;
	friend class cHooks;
	friend class cStorage;
	friend class cMonster;
	friend class cMonsterServer;
	friend class cObject;
	friend class cAvatar;
	friend class cPets;
	friend class cCorpse;
	friend class cWorldObject;
	friend class cFood;
	friend class cFoodContainer;
	friend class cMerchantSign;
	friend class cScrolls;
	friend class cHealingKits;
	friend class cGems;
	friend class cBooks;
	friend class cManaStones;
	friend class cLockpicks;
	friend class cWands;
	friend class cTradeSkillMats;
	friend class cSpellComps;
	friend class cAmmo;
	friend class cSalvage;
	friend class cPyreals;
	friend class cJewelry;
	friend class cHealingCon;
	friend class cWandCon;
	friend class cCompCon;
	friend class cTradeNotes;
	friend class cPlants;
	friend class cWeapon;
	friend class cClothes;
	friend class cPack;
	friend class cShield;
	friend class cFoci;
	friend class TreasureGen;
	friend class cMisc;
	friend class cWarSpell;
	friend class SimpleAI;
	friend class cEnchantment;

public:
	cClient	( SOCKADDR_IN& saSockAddr, BOOL fAddToHash = TRUE )	
		:	m_dwF7B0Sequence( 0 ),
			m_dwLastRecvTime( 0 ),
			m_bCharState	( 1 ),
			m_bWorldState	( 0 ),
			m_saSockAddr	( saSockAddr ),
			m_pcAvatar		( NULL ),
			m_pcPrev		( NULL ),
			m_pcNext		( NULL )
	{
		m_PacketPipe_WS.Initialize( );
		m_PacketPipe_CS.Initialize( );

		if ( fAddToHash )
			Hash_Add( this );
	}

	cClient	( )	{}
	~cClient( ) 
	{		
		const WORD wSum = Hash_AddrSum( m_saSockAddr );
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext; //crashes attacking; 0xC0000005 access error
		else			m_lpcHashTable[wSum] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}

	static inline void Hash_Load( )
	{
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) );
	}

	static inline cClient *Hash_New( SOCKADDR_IN& saSockAddr )
	{
		cClient *pcClient = Hash_Find( saSockAddr );
		if ( pcClient )
			return pcClient;
		
		return new cClient( saSockAddr );
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cClient *pcClient );
	static cClient	*FindClient	( DWORD dwGUID );
	static cClient	*FindClient	( char *szName );
	static cAvatar	*FindAvatar	( DWORD dwGUID );
	static cAvatar	*FindAvatar	( char *szName );

	static inline void SendOffAllPackets( )
	{
		cClient *pcClient;
		for ( int i = 0; i < 1020; ++i )
		{
			pcClient = m_lpcHashTable[i];
			while ( pcClient )
			{
				pcClient->SendQueuedPackets( );
				pcClient = pcClient->m_pcNext;
			}
		}
	}

	static inline void SendToAllClients( cMessage& cmData, WORD wGroup )
	{	
		cClient *pcClient;
		for ( int i = 0; i < 1020; ++i )
		{
			pcClient = m_lpcHashTable[i];
			while ( pcClient )
			{
				if ( pcClient->m_pcAvatar )
					pcClient->AddPacket( WORLD_SERVER, cmData, wGroup );

				pcClient = pcClient->m_pcNext;
			}
		}
	}

	static inline void SendToAllClients( BYTE *pbData, WORD wSize, WORD wGroup )
	{	
		cClient *pcClient;
		for ( int i = 0; i < 1020; ++i )
		{
			pcClient = m_lpcHashTable[i];
			while ( pcClient )
			{
				if ( pcClient->m_pcAvatar )
					pcClient->AddPacket( WORLD_SERVER, pbData, wSize, wGroup );

				pcClient = pcClient->m_pcNext;
			}
		}
	}

	static inline void SendToAllOtherClients( cClient *pcClientOrg, BYTE *pbData, WORD wSize, WORD wGroup )
	{
		cClient *pcClient;
		for ( int i = 0; i < 1020; ++i )
		{
			pcClient = m_lpcHashTable[i];
			while ( pcClient )
			{
				if ( pcClient->m_pcAvatar && pcClientOrg != pcClient )
					pcClient->AddPacket( WORLD_SERVER, pbData, wSize, wGroup );

				pcClient = pcClient->m_pcNext;
			}
		}
	}

	static inline void SendToAllOtherClients( cClient *pcClientOrg, cMessage& cmData, WORD wGroup )
	{
		cClient *pcClient;
		for ( int i = 0; i < 1020; ++i )
		{
			pcClient = m_lpcHashTable[i];
			while ( pcClient )
			{
				if ( pcClient->m_pcAvatar && pcClientOrg != pcClient )
					pcClient->AddPacket( WORLD_SERVER, cmData, wGroup );

				pcClient = pcClient->m_pcNext;
			}
		}
	}

	inline void AddPacket( SERVER eServer, BYTE *pbData, WORD wSize, WORD wGroup )
	{
		if		( eServer == WORLD_SERVER )	m_PacketPipe_WS.AddPacket( pbData, wSize, wGroup );
		else if ( eServer == CHAR_SERVER )	m_PacketPipe_CS.AddPacket( pbData, wSize, wGroup );
	}
	inline void AddPacket( SERVER eServer, cMessage& cmPacket, WORD wGroup )
	{
		if		( eServer == WORLD_SERVER ) m_PacketPipe_WS.AddPacket( cmPacket, wGroup );
		else if	( eServer == CHAR_SERVER )	m_PacketPipe_CS.AddPacket( cmPacket, wGroup );
	}
	inline void SendQueuedPackets( )
	{
		m_PacketPipe_WS.SendQueuedPackets( m_saSockAddr );
		m_PacketPipe_CS.SendQueuedPackets( m_saSockAddr );
	}

	void ProcessPacket_CS( cRecvPacket *pcRecvPacket );
	void ProcessPacket_WS( cRecvPacket *pcRecvPacket );

	SOCKADDR_IN		m_saSockAddr;
	cClient *m_pcNext;
	cClient *m_pcPrev;
	//char	m_szAccountName[45];

private:
	static cClient *m_lpcHashTable[1020];

	// Character Server functions
	//========================================
	inline void SendPacketF7B8	( );
	void		SendMOTD		( );
	inline void SendPacketF7C7	( );
	void		SendAddress		( char *szAddr, WORD wPort );
	void		SendAvatarList	( );
	void		SendDeleteAck	( );
	//========================================

	// World Server functions
	//========================================
	inline void SendPacket100( );
	inline void SendPacket400( );
	//========================================

	

	static inline void Hash_Add( cClient *pcClient )
	{
		const WORD wSum = Hash_AddrSum( pcClient->m_saSockAddr );
		if ( !m_lpcHashTable[wSum] )
			m_lpcHashTable[wSum] = pcClient;
		else
		{
			pcClient->m_pcNext				= m_lpcHashTable[wSum];
			m_lpcHashTable[wSum]->m_pcPrev	= pcClient;
			m_lpcHashTable[wSum]			= pcClient;
		}
	}

	static inline cClient *Hash_Find( SOCKADDR_IN& saSockAddr )
	{
		const WORD wSum = Hash_AddrSum( saSockAddr );
		cClient *pcClient = m_lpcHashTable[wSum];
		while ( pcClient )
		{
			if ( pcClient->CompareAddress( saSockAddr ) )	return pcClient;
			else											pcClient = pcClient->m_pcNext;
		}
		return NULL;
	}

	static inline WORD Hash_AddrSum( SOCKADDR_IN& saSockAddr )
	{
		return	HIBYTE( HIWORD( saSockAddr.sin_addr.S_un.S_addr ) ) + LOBYTE( HIWORD( saSockAddr.sin_addr.S_un.S_addr ) ) +
				HIBYTE( LOWORD( saSockAddr.sin_addr.S_un.S_addr ) ) + LOBYTE( LOWORD( saSockAddr.sin_addr.S_un.S_addr ) );
	}
	
	inline BOOL CompareAddress( SOCKADDR_IN& saSockAddr )
	{
		if	(	saSockAddr.sin_addr.S_un.S_addr == m_saSockAddr.sin_addr.S_un.S_addr && 
				saSockAddr.sin_port == m_saSockAddr.sin_port )
			return TRUE;
		else																			
			return FALSE;
	}

	DWORD	m_dwF7B0Sequence;
	DWORD	m_dwLastRecvTime;
	BYTE	m_bCharState;
	BYTE	m_bWorldState;
	WORD	m_wAccountNameLength;
	char	m_szAccountName[45];
	DWORD	m_dwAccountID;

	cAvatar	*m_pcAvatar;

	cPacketPipe< cCharacterServer >	m_PacketPipe_CS;
	cPacketPipe< cWorldServer >		m_PacketPipe_WS;
	std::vector< cAvatarList >		m_AvatarList;
};

/*Karki's Mindless Shove
void cClient::ServerMessage( DWORD dwColor, cClient *pcClient, char *szMessage, ... )
{
	char	szTextBuffer[1024];

	va_list	valMaker;
	
	va_start( valMaker, szMessage );
	wvsprintf( szTextBuffer, szMessage, valMaker );

	cMessage cmSM;
	cmSM << 0xF62CL << szTextBuffer << dwColor;

	if ( pcClient == NULL )
		cClient::SendToAllClients( cmSM, 4 );
	else
		pcClient->AddPacket( WORLD_SERVER, cmSM, 4 );
}
End Karki's Madness*/

//Inline Functions
//================

inline void cClient::SendPacketF7B8( )
{
	BYTE MSG_F7B8[44] = { //Dec 28 //June 44	
	/*
	// Dec [28]
		0xB8, 0xF7, 0x00, 0x00, 0x2D, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE7, 0x04, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	*/

	// June	  [44]
		0xB8, 0xF7, 0x00, 0x00, 0x90, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x05, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x07, 0x00, 0x00, 0x00, 
		0x00, 0x80, 0x01, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x07, 0x00,
	};

	AddPacket( CHAR_SERVER, (BYTE *)MSG_F7B8, 44, 7 ); //Dec 28,7 //June 44,7
	AddPacket( CHAR_SERVER, (BYTE *)MSG_F7B8, 44, 7 );
}

inline void cClient::SendPacketF7C7( )
{
	DWORD dwCharSel = 0x0000F7C7;
	AddPacket( CHAR_SERVER, (BYTE *)&dwCharSel, 4, 4 );
}

inline void cClient::SendPacket100( )
{
	m_PacketPipe_WS.CalcCRC( SERVER_Packet100, 70 );

	sendto( m_PacketPipe_WS.m_Socket, (char *)SERVER_Packet100, 90, NULL, (SOCKADDR *)&m_saSockAddr, sizeof( SOCKADDR ) );
}

inline void cClient::SendPacket400( )
{
	m_PacketPipe_WS.CalcCRC( SERVER_Packet400, 8 );

	sendto( m_PacketPipe_WS.m_Socket, (char *)SERVER_Packet400, 28, NULL, (SOCKADDR *)&m_saSockAddr, sizeof( SOCKADDR ) );
}

#endif	// #ifndef __CLIENT_H