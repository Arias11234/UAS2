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

/* Special thanks to David Simpson for his work on the cell.dat and portal.dat extraction code */

/**
 *	@file WorldManager.h
 */

#ifndef __WORLDMANAGER_H
#define __WORLDMANAGER_H

#pragma warning(disable:4786)	//warning: identifier was truncated to '255' characters in the browser information

#include <winsock2.h>
#include <algorithm>
#include <stack>

#include "Avatar.h"
#include "Client.h"
#include "Object.h"

#define CELLSECSIZE		64
#define NUMFILELOC		0x03F
#define ROOTDIRPTRLOC	0x148

class cLandBlock
{
	friend class cWorldManager;

public:
	cLandBlock( WORD wLandBlock, BOOL fAddToHash = TRUE )
		:	m_wLandBlock( wLandBlock ),
			m_pcPrev	( NULL ),
			m_pcNext	( NULL )
	{
		if ( fAddToHash )
			Hash_Add( this );
		Load_LandBlockZ( this );
	}
	
	~cLandBlock( )
	{
		const WORD wSum = HIBYTE( m_wLandBlock ) + LOBYTE( m_wLandBlock );
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[wSum] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}
	
	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline void Hash_Unload( )
	{
		cLandBlock *pcLB, *pcPrevLB;
		for ( int i = 0; i < 510; ++i )
		{
			pcLB = m_lpcHashTable[i];
			while ( pcLB )
			{
				pcPrevLB	= pcLB;
				pcLB		= pcLB->m_pcNext;
				
				while ( !pcPrevLB->m_lstObjects.empty( ) )
				{
					SAFEDELETE( pcPrevLB->m_lstObjects.front( ) )
					pcPrevLB->m_lstObjects.pop_front( );
				}

				SAFEDELETE( pcPrevLB )
			}
		}
	}

	static inline cLandBlock *Hash_New( WORD wLandBlock )
	{
		cLandBlock *pcLB = Hash_Find( wLandBlock );
		if ( pcLB )
			return pcLB;
		
		return new cLandBlock( wLandBlock );
	}

	static inline void Hash_Remove( cLandBlock *pcLB )
	{
		SAFEDELETE( pcLB )
	}

	static inline cLandBlock *Hash_Find( WORD wLandBlock )
	{
		const WORD wSum = HIBYTE( wLandBlock ) + LOBYTE( wLandBlock );
		cLandBlock *pcLB = m_lpcHashTable[wSum];
		while ( pcLB )
		{ //crash occurs here
			if ( pcLB->m_wLandBlock == wLandBlock )	return pcLB;
			else  pcLB = pcLB->m_pcNext;
			
		}
		//UpdateConsole(" Landblock hash checked...%d\n",&wLandBlock);
		return NULL;
	}

	WORD m_wLandBlock;
	float m_flLandBlockZ[9][9];

private:
	static cLandBlock *m_lpcHashTable[510];

	static inline void Hash_Add( cLandBlock *pcLB )
	{
		const WORD wSum = HIBYTE( pcLB->m_wLandBlock ) + LOBYTE( pcLB->m_wLandBlock );
		if ( !m_lpcHashTable[wSum] )
			m_lpcHashTable[wSum] = pcLB;
		else
		{
			pcLB->m_pcNext					= m_lpcHashTable[wSum];
			m_lpcHashTable[wSum]->m_pcPrev	= pcLB;
			m_lpcHashTable[wSum]			= pcLB;
		}
	}

	/**
	 *	Loads the cell.dat information for the specified file.
	 *
	 *	Calls FetchFilePos to find a landblock's data's position.
	 *	Calls FetchFile to fetch a landblock's data from its found position.
	 *
	 *	@param *pcLB - A pointer to the landblock the data of which is to be loaded from the cell.dat.
	 */
	static inline void Load_LandBlockZ( cLandBlock *pcLB )
	{
		FILE  *inFile;
		int read;
		UINT rootDirPtr, id, filePos, len;
		UCHAR *buf, *zData;

		inFile = fopen("cell.dat", "rb");
		if (inFile == NULL)
		{
			UpdateConsole(" Cell.dat: Open failed.\r\n");
			return;
		}
		read = fseek(inFile, ROOTDIRPTRLOC, SEEK_SET);
		if (read != 0)
		{
			UpdateConsole(" Cell.dat: Read error.\r\n");
			fclose(inFile);
			return;
		}
		
		read = fread(&rootDirPtr, sizeof(UINT), 1, inFile);
		if (read != 1)
		{
			UpdateConsole(" Cell.dat: End of file reached.\r\n");
			fclose(inFile);
			return;
		}
		
		id = pcLB->m_wLandBlock * 0x10000 + 0xFFFF;
		
		if (!cLandBlock::FetchFilePos(inFile, rootDirPtr, id, &filePos, &len))
		{
			UpdateConsole(" Cell.dat: File not found.\r\n");
			fclose(inFile);
			return;
		}

		buf = (UCHAR *)malloc(len);
		if (!cLandBlock::FetchFile(inFile, filePos, len, buf))
		{
			free(buf);
			fclose(inFile);
			return;
		}

		zData = &buf[170];
		char Command[100];
		for (int x = 0; x < 9; x++)
		{
			for (int y = 0; y < 9; y++)
			{
				pcLB->m_flLandBlockZ[x][y] = zData[x * 9 + y];
				sprintf(Command,"zData: %f\r\n", pcLB->m_flLandBlockZ[x][y]);
				//UpdateConsole(Command);
			}
		}

		free(buf);
		fclose(inFile);
	}

	/**
	 *	Finds the location of a landblock's data in the cell.dat.
	 *
	 *	@param &inFile - The address of the file/directory to search (the cell.dat).
	 *	@param disPos - The beginning position of the file pointer (in BYTEs).
	 *	@param id - The landblock to be loaded from the cell.dat.
	 *	@param *filePos - A pointer to the variable that should receive the position of the landblock data.
	 *	@param *len - A pointer to the variable that should receive the length of the landblock data.
	 */
	static inline int FetchFilePos(FILE *inFile, UINT dirPos, UINT id, UINT *filePos, UINT *len)
	{
		UINT dir[4 * CELLSECSIZE];
		UINT i;
		UINT numFiles;
		int read;

		while (1)
		{
			if (dirPos == 0)
			{
				UpdateConsole(" Cell.dat: NULL directory entry found.\r\n");
				return 0;
			}

			read = fseek(inFile, dirPos, SEEK_SET);
			if (read != 0)
			{
				UpdateConsole(" Cell.dat: Sector is beyond end of file.\r\n");
				return 0;
			}

			read = fread(dir, sizeof(UINT), CELLSECSIZE, inFile);
			if (read != CELLSECSIZE)
			{
				UpdateConsole(" Cell.dat: Sector doesn't contain enough words.\r\n");
				return 0;
			}

			dirPos = dir[0];

			if (dirPos != 0)
			{
				read = fseek(inFile, dirPos, SEEK_SET);
				if (read != 0)
				{
					UpdateConsole(" Cell.dat: Seek is beyond end of file.\r\n");
					return 0;
				}

				read = fread(&dirPos, sizeof(UINT), 1, inFile);
				if (read != 1)
				{
					UpdateConsole(" Cell.dat: Sector does not exist.\r\n");
					return 0;
				}

				read = fread(&dir[CELLSECSIZE], sizeof(UINT), CELLSECSIZE - 1, inFile);
				if (read != CELLSECSIZE - 1)
				{
					UpdateConsole(" Cell.dat: Sector doesn't contain enough words.\r\n");
					return 0;
				}
			}

			if (dirPos != 0)
			{
				read = fseek(inFile, dirPos, SEEK_SET);
				if (read != 0)
				{
					UpdateConsole(" Cell.dat: Seek is beyond end of file.\r\n");
					return 0;
				}

				read = fread(&dirPos, sizeof(UINT), 1, inFile);
				if (read != 1)
				{
					UpdateConsole(" Cell.dat: Sector does not exist.\r\n");
					return 0;
				}

				read = fread(&dir[CELLSECSIZE * 2 - 1], sizeof(UINT), CELLSECSIZE - 1, inFile);
				if (read != CELLSECSIZE - 1)
				{
					UpdateConsole(" Cell.dat: Sector doesn't contain enough words.\r\n");
					return 0;
				}
			}
			
			if (dirPos != 0)
			{
				read = fseek(inFile, dirPos, SEEK_SET);
				if (read != 0)
				{
					UpdateConsole(" Cell.dat: Seek is beyond end of file\r\n");
					return 0;
				}

				read = fread(&dirPos, sizeof(UINT), 1, inFile);
				if (read != 1)
				{
					UpdateConsole(" Cell.dat: Sector does not exist.\r\n");
					return 0;
				}

				read = fread(&dir[CELLSECSIZE * 3 - 2], sizeof(UINT), CELLSECSIZE - 1, inFile);
				if (read != CELLSECSIZE - 1)
				{
					UpdateConsole(" Cell.dat: Sector doesn't contain enough words\r\n");
					return 0;
				}
			}

			numFiles = dir[NUMFILELOC];
			if (numFiles >= NUMFILELOC)
			{
				UpdateConsole(" Cell.dat: Number of files exceeds directory entries.\r\n");
				return 0;
			}

			i = 0;
			while ((i < numFiles) && (id > dir[i * 3 + NUMFILELOC + 1]))
			{
				i++;
			}
			if (i < numFiles)
			{
				if (id == dir[i*3 + NUMFILELOC + 1])
				{
					*filePos = dir[i * 3 + NUMFILELOC + 2];
					*len = dir[i * 3 + NUMFILELOC + 3];
					return 1;
				}
			}
			
			if (dir[1] == 0)
			{
				filePos = 0;
				len = 0;
				return 0;
			}

			dirPos = dir[i + 1];
		}
		return 0;
	}

	/**
	 *	Fetches landblock information from the cell.dat.
	 *
	 *	@param *inFile - A pointer to the file/directory to search (the cell.dat).
	 *	@param filePos - The variable that should receive the position of the landblock data.
	 *	@param len - The variable that should receive the length of the landblock data.
	 *	*@param &buf - A pointer to the buffer that should receive the landblock data.
	 */
	static inline int FetchFile(FILE *inFile, UINT filePos, UINT len, UCHAR *buf)
	{
		int read, doChain;
		UINT sec[CELLSECSIZE];

		if (filePos == 0)
		{
			UpdateConsole(" Cell.dat: Null file pointer found.\r\n");
			return 0;
		}

		doChain = 1;
		while (doChain)
		{
			read = fseek(inFile, filePos, SEEK_SET);
			if (read != 0)
			{
				UpdateConsole(" Cell.dat: Seek failed.\r\n");
				return 0;
			}

			read = fread(sec, sizeof(UINT), CELLSECSIZE, inFile);
			if (read != CELLSECSIZE)
			{
				UpdateConsole(" Cell.dat: Sector doesn't contain enough words.\r\n");
				return 0;
			}

			filePos = sec[0] * 0x7FFFFFFF;

			if (len > (CELLSECSIZE - 1) * sizeof(UINT))
			{
				memcpy(buf, &sec[1], (CELLSECSIZE - 1) * sizeof(UINT));
				buf += (CELLSECSIZE - 1) * sizeof(UINT);
				len -= (CELLSECSIZE - 1) * sizeof(UINT);
			}
			else
			{
				memcpy(buf, &sec[1], len);
				len = 0;
			}

			if (filePos == 0)
				doChain = 0;
		}

		return 1;
	}

	std::list< cClient * >	m_lstClients;
	std::list< cClient * >	m_lstFocusedClients;
	std::list< cObject * >	m_lstObjects;
	cLandBlock				*m_pcPrev;
	cLandBlock				*m_pcNext;
};	

class cWorldManager
{
	friend class cMasterServer;

public:
	cWorldManager	( )	{}
	~cWorldManager	( )	{}

	typedef std::vector< ConfirmPanel >::iterator iterConfirmPanel_lst;

	static void Load( );

	static void Unload( );

	static BOOL RemoveClient	( cClient *pcClient, BOOL fRemoveAvatar = TRUE, BOOL fDeleteAvatar = TRUE );
	static BOOL AddClient		( cClient *pcClient, BOOL fSpawnAvatar = TRUE );
	static BOOL MoveAvatar		( cClient *pcClient, cLocation& NewLoc, WORD wAnim = 0, float flPlaySpeed = 1.0f );
	static BOOL TeleportAvatar	( cClient *pcClient, cLocation& NewLoc );

	static BOOL RemoveObject	( cObject *pcObject, BOOL fRemoveObject = TRUE, BOOL fDeleteObject = TRUE );

	static BOOL AddObject		( cObject *pcObject, BOOL fSpawnObject = TRUE );
	static BOOL MoveAddObject	( cObject *pcObject ); // Object changes landblock location
	static BOOL MoveRemObject	( cObject *pcObject ); // Object changes landblock location

	static void RemoveAllObjects( );

	static inline void SendToAllInLandBlock( cLocation& Loc, cMessage& cmPacket, WORD wGroup )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( Loc.m_dwLandBlock ) );

		if ( pcLB )
			SendToAllInLandBlock( pcLB, cmPacket, wGroup );
	}

	static inline void SendToAllInFocus( cLocation& Loc, cMessage& cmPacket, WORD wGroup )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( Loc.m_dwLandBlock ) );

		if ( pcLB )
			SendToAllInFocus( pcLB, cmPacket, wGroup );
	}

	static inline void SendToAllWithin( BYTE bDistance, cLocation& Loc, cMessage& cmPacket, WORD wGroup )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( Loc.m_dwLandBlock ) );

		if ( pcLB )
			SendToAllWithin( bDistance, pcLB, cmPacket, wGroup );
	}

	static inline void SendToOthersInLandBlock( cLocation& Loc, cClient *pcClient, cMessage& cmPacket, WORD wGroup )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( Loc.m_dwLandBlock ) );

		if ( pcLB )
			SendToOthersInLandBlock( pcLB, pcClient, cmPacket, wGroup );
	}

	static inline void SendToOthersInFocus( cLocation& Loc, cClient *pcClient, cMessage& cmPacket, WORD wGroup )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( Loc.m_dwLandBlock ) );

		if ( pcLB )
			SendToOthersInFocus( pcLB, pcClient, cmPacket, wGroup );
	}

	static inline void SendToOthersWithin( BYTE bDistance, cLocation& Loc, cClient *pcClient, cMessage& cmPacket, WORD wGroup )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( Loc.m_dwLandBlock ) );

		if ( pcLB )
			SendToOthersWithin( bDistance, pcLB, pcClient, cmPacket, wGroup );
	}

	static inline cObject *GetCollisionPortal( cLocation OldLoc, cLocation NewLoc )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( OldLoc.m_dwLandBlock ) );


		for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); ++itObject )

			{
				//UpdateConsole(" Item Checked: %s Type: %d\r\n", (*itObject)->Name(), (*itObject)->GetType());
				if ( (*itObject)->GetType() == 1000 )
				{
					float flDist = cPhysics::Get3DLineDistance( OldLoc, NewLoc, (*itObject)->m_Location );
					float flOldDist = cPhysics::Get3DRange( OldLoc, (*itObject)->m_Location );
					/*
					char szPortalInfo[100];
					sprintf( szPortalInfo, "Portal Found:  %s, Range: %f, OldRange: %f\r\n", (*itObject)->Name(), flDist, flOldDist );
					UpdateConsole((char *)szPortalInfo);
					*/
					if ( flDist < .05  && flOldDist >= .05)
						return *itObject;
				}
			}
		
		return NULL;
	}

	static inline cObject *GetCollisionMonster( DWORD SpellGUID, cLocation SpellLoc )
	{
		cLandBlock *pcLB = cLandBlock::Hash_Find( HIWORD( SpellLoc.m_dwLandBlock ) );

		for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); ++itObject )
		{
			//UpdateConsole("Item Checked: %s Type: %d\r\n", (*itObject)->Name(), (*itObject)->GetType());
			if ( SpellGUID != (*itObject)->GetGUID() ) //(*itObject)->GetType() == 0 )
			{
				
				float flDist = cPhysics::GetRange( SpellLoc, (*itObject)->m_Location );
				
				
				char szPortalInfo[100];
				sprintf( szPortalInfo, "Collision Target:  %s, Range: %f\r\n", (*itObject)->Name(), flDist );
				//UpdateConsole((char *)szPortalInfo);
				
				if ( flDist < .05 && (*itObject)->m_dwObjectFlags1 & 0x00000010 && (*itObject)->m_fDeadOrAlive == true)
					return *itObject;
			}
		}
		return NULL;
	}

	static inline cObject *FindObject( DWORD dwGUID )
	{
		cLandBlock *pcLB;
		for ( int i = 0; i < 510; ++i )
		{
			pcLB = cLandBlock::m_lpcHashTable[i];
			while ( pcLB )
			{
				for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); ++itObject )
				{
					if ( dwGUID == (*itObject)->GetGUID( ) )
						return *itObject;
				}
				pcLB = pcLB->m_pcNext;
			}
		}
		return NULL;
	}

	//Test to find corpses
	static inline cCorpse *FindCorpse( DWORD dwGUID )
	{
		cLandBlock *pcLB;
		for ( int i = 0; i < 510; ++i )
		{
			pcLB = cLandBlock::m_lpcHashTable[i];
			while ( pcLB )
			{
				for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); ++itObject )
				{
					if ( dwGUID == (*itObject)->GetGUID( ) )
						return reinterpret_cast<cCorpse *>(*itObject);
				}
				pcLB = pcLB->m_pcNext;
			}
		}
		return NULL;
	}

	//Find avatars
	//Fix: Search within landblock focus.
	static inline cAvatar *FindAvatar( DWORD dwGUID )
	{
		cLandBlock *pcLB;
		for ( int i = 0; i < 510; ++i )
		{
			pcLB = cLandBlock::m_lpcHashTable[i];
			while ( pcLB )
			{
				for ( iterClient_lst itClient = pcLB->m_lstClients.begin( ); itClient != pcLB->m_lstClients.end( ); ++itClient )
				{
//						char szCharInfo[100];
//						sprintf( szCharInfo, "Client Avatar %d\n",(*itClient)->m_pcAvatar->m_dwGUID );
//						UpdateConsole ((char *)szCharInfo);
					if ( dwGUID == (*itClient)->m_pcAvatar->m_dwGUID )
						return reinterpret_cast<cAvatar *>((*itClient)->m_pcAvatar);
				}
				pcLB = pcLB->m_pcNext;
			}
		}
		return NULL;
	}

	//k109:  Added this to try and implement item ID in npc inventory.
	static inline cNPC *FindNPC( DWORD dwGUID )
	{
		cLandBlock *pcLB;
		for ( int i = 0; i < 510; ++i )
		{
			pcLB = cLandBlock::m_lpcHashTable[i];
			while ( pcLB )
			{
				for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); ++itObject )
				{
					if ( dwGUID == (*itObject)->GetGUID( ) )
						return reinterpret_cast<cNPC *>(*itObject);
				}
				pcLB = pcLB->m_pcNext;
			}
		}
		return NULL;
	}

	//TODO: Create list to hold recycled ObjectGUIDs (GUIDs of objects that have ceased existing).
	//Preference would be given to using rec	ycled GUIDs from this list over creating new ones.
	static inline UINT		NewGUID_Avatar		( )					{ return ++m_dwGUIDCycle_Avatar; }
	static inline UINT		NewGUID_Object		( )					{ return ++m_dwGUIDCycle_Object; }
	static inline void		AddUnusedAvatarGUID	( DWORD dwGUID )	{ m_lstUnusedAvatarGUIDs.push_back(dwGUID); }
	static inline void		AddUnusedObjectGUID	( DWORD dwGUID )	{ m_lstUnusedObjectGUIDs.push_back(dwGUID); }
	static inline DWORD		GetUnusedAvatarGUID	( )					{ return m_lstUnusedAvatarGUIDs.back(); }
	static inline DWORD		GetUnusedObjectGUID	( )					{ return m_lstUnusedObjectGUIDs.back(); }

	static char		g_szAccessFile[MAX_PATH+20];

	static inline	DWORD CurrentConfirmSeq	( )		{ return m_dwConfirmSequence; }
	static inline	DWORD NewConfirmSeq		( )		{ return ++m_dwConfirmSequence; }

	static void		AddPendingConfirm		( DWORD sequence, DWORD type, std::string szText, DWORD senderGUID, DWORD receiptGUID = NULL );
	static void		FindPendingConfirm		( DWORD dwSeq, DWORD dwReply );

private:
	static inline void RemoveAvatar( cClient *pcClient, cLandBlock *pcLB, WORD wAnim = 0, float flPlaySpeed = 1.0f )
	{
		if ( wAnim )
		{
			pcClient->m_pcAvatar->LogoutCharacter();
			cMessage cmAnim = pcClient->m_pcAvatar->Animation( wAnim, flPlaySpeed );
			SendToAllInFocus( pcLB, cmAnim, 3 );
		}

		cMessage cmRemModel;
		cmRemModel << 0xF747L << pcClient->m_pcAvatar->GetGUID( ) << DWORD( pcClient->m_pcAvatar->m_wNumLogins );
		SendToOthersInFocus( pcLB, pcClient, cmRemModel, 3 );
	}

	static inline void RemoveObject( cObject *pcObject, cLandBlock *pcLB )
	{
		cMessage cmRemModel;
		cmRemModel << 0xF747L << pcObject->GetGUID( ) << DWORD( pcObject->m_wNumLogins );
		SendToAllInFocus( pcLB, cmRemModel, 3 );
	}
/*
	static inline void RemoveObjectContainer( cClient *pcClient, cObject *pcObject, DWORD dwContainer )
	{
		cMessage cmRemoveItem;
		cmRemoveItem << 0x0024L << pcObject->GetGUID( );
		pcClient->AddPacket(WORLD_SERVER,cmRemoveItem,4);
	}
*/
	static inline void SpawnAvatar( cClient *pcClient, cLandBlock *pcLB )
	{
		cMessage cmLogin;

		*((DWORD *)&MSG_00000002[4]) = pcClient->m_pcAvatar->GetGUID( );
		cmLogin.CannedData( MSG_00000002, sizeof( MSG_00000002 ) );

		// Receive the create messages for all other object in focus range.
		ReceiveAllObjectsInFocus( pcClient, pcLB );
		// Send this avatar's create and materialize messages to others.
		/*
		BOOL fContinue;

		do
		{
			cMessage cmChildren = pcClient->m_pcAvatar->CreateChildren( fContinue );
			if( fContinue )
				SendToAllInFocus( pcLB, cmChildren, 3 );
		} while ( fContinue );
		*/

		SendToOthersInFocus( pcLB, pcClient, pcClient->m_pcAvatar->CreatePacket( ), 3 );
		SendToAllInFocus( pcLB, cmLogin, 3 );
		
	}

	static inline void SpawnObject( cObject *pcObject, cLandBlock *pcLB, cClient *pcClient = NULL )
	{
		if( pcClient )
			SendToOthersInFocus( pcLB, pcClient, pcObject->CreatePacket( ), 3 );
		else
			SendToAllInFocus( pcLB, pcObject->CreatePacket( ), 3 );
	}

	static inline void SendToAllInLandBlock( cLandBlock *pcLB, cMessage& cmPacket, WORD wGroup )
	{
		for ( iterClient_lst itClient = pcLB->m_lstClients.begin( ); itClient != pcLB->m_lstClients.end( ); ++itClient )
			(*itClient)->AddPacket( WORLD_SERVER, cmPacket, wGroup );
	}

	static inline void SendToAllInFocus( cLandBlock *pcLB, cMessage& cmPacket, WORD wGroup )
	{
		SendToAllInLandBlock( pcLB, cmPacket, wGroup );
		for ( iterClient_lst itClient = pcLB->m_lstFocusedClients.begin( ); itClient != pcLB->m_lstFocusedClients.end( ); ++itClient )
			(*itClient)->AddPacket( WORLD_SERVER, cmPacket, wGroup );

	}
	static inline void SendToAllWithin( BYTE bDistance, cLandBlock *pcLB, cMessage& cmPacket, WORD wGroup )
	{
		const WORD wDiameter = (bDistance * 2 + 1) * (bDistance * 2 + 1);
		WORD *pwLBs = new WORD[wDiameter];
		LandBlocksWithin( bDistance, pcLB->m_wLandBlock, pwLBs );
		
		SendToAllInLandBlock( pcLB, cmPacket, wGroup );
		for ( int i = 0; i < wDiameter - 1; ++i )
		{
			pcLB = cLandBlock::Hash_Find( pwLBs[i] );
			if ( pcLB )
				SendToAllInLandBlock( pcLB, cmPacket, wGroup );
		}
		SAFEDELETE_ARRAY( pwLBs )
	}

	static inline void SendToOthersInLandBlock( cLandBlock *pcLB, cClient *pcClient, cMessage& cmPacket, WORD wGroup )
	{
		for ( iterClient_lst itClient = pcLB->m_lstClients.begin( ); itClient != pcLB->m_lstClients.end( ); ++itClient )
		{
			if ( pcClient != *itClient )
				(*itClient)->AddPacket( WORLD_SERVER, cmPacket, wGroup );
		}
	}

	static inline void SendToOthersInFocus( cLandBlock *pcLB, cClient *pcClient, cMessage& cmPacket, WORD wGroup )
	{
		SendToOthersInLandBlock( pcLB, pcClient, cmPacket, wGroup );

		for ( iterClient_lst itClient = pcLB->m_lstFocusedClients.begin( ); itClient != pcLB->m_lstFocusedClients.end( ); ++itClient )
		{
			if ( pcClient != *itClient )
			{
				try
				{
					(*itClient)->AddPacket( WORLD_SERVER, cmPacket, wGroup );
				}
				catch ( char * str )
				{
					UpdateConsole(" Exception occured:  %s", str);
				}
			}
		}
	}

	static inline void SendToOthersWithin( BYTE bDistance, cLandBlock *pcLB, cClient *pcClient, cMessage& cmPacket, WORD wGroup )
	{
		const WORD wDiameter = (bDistance * 2 + 1) * (bDistance * 2 + 1);
		WORD *pwLBs = new WORD[wDiameter];
		LandBlocksWithin( bDistance, pcLB->m_wLandBlock, pwLBs );
		
		SendToOthersInLandBlock( pcLB, pcClient, cmPacket, wGroup );
		for ( int i = 0; i < wDiameter - 1; ++i )
		{
			pcLB = cLandBlock::Hash_Find( pwLBs[i] );
			if ( pcLB )
				SendToOthersInLandBlock( pcLB, pcClient, cmPacket, wGroup );
		}
		SAFEDELETE_ARRAY( pwLBs )
	}

	static inline void AddClient( cLandBlock *pcLB, cClient *pcClient )	
	{ 
		iterClient_lst itClient = std::find( pcLB->m_lstClients.begin( ), pcLB->m_lstClients.end( ), pcClient );
		
		if ( itClient == pcLB->m_lstClients.end( ) )
			pcLB->m_lstClients.push_back( pcClient ); 
	}
	
	static inline void AddFocus( cLandBlock *pcLB, cClient *pcClient )	
	{ 
		iterClient_lst itClient = std::find( pcLB->m_lstFocusedClients.begin( ), pcLB->m_lstFocusedClients.end( ), pcClient );
		
		if ( itClient == pcLB->m_lstFocusedClients.end( ) )
			pcLB->m_lstFocusedClients.push_back( pcClient ); 
	}
	
	static inline void AddObject( cLandBlock *pcLB, cObject *pcObject ) 
	{
		iterObject_lst itObject = std::find( pcLB->m_lstObjects.begin( ), pcLB->m_lstObjects.end( ), pcObject );

		if ( itObject == pcLB->m_lstObjects.end( ) )
			pcLB->m_lstObjects.push_back( pcObject ); 
	}
	
	static inline BOOL RemClient( cLandBlock *pcLB, cClient *pcClient )
	{
		iterClient_lst itClient = std::find( pcLB->m_lstClients.begin( ), pcLB->m_lstClients.end( ), pcClient );

		if ( itClient == pcLB->m_lstClients.end( ) )
			return FALSE;

		pcLB->m_lstClients.erase( itClient );
		if ( pcLB->m_lstClients.empty( ) && pcLB->m_lstFocusedClients.empty( ) && pcLB->m_lstObjects.empty( ) )
			cLandBlock::Hash_Remove( pcLB );

		return TRUE;
	}

	static inline BOOL RemFocus( cLandBlock *pcLB, cClient *pcClient)
	{
		iterClient_lst itClient = std::find( pcLB->m_lstFocusedClients.begin( ), pcLB->m_lstFocusedClients.end( ), pcClient );

		if ( itClient == pcLB->m_lstFocusedClients.end( ) )
			return FALSE;
		
		pcLB->m_lstFocusedClients.erase( itClient );

		if ( pcLB->m_lstClients.empty( ) && pcLB->m_lstFocusedClients.empty( ) && pcLB->m_lstObjects.empty( ) )
			cLandBlock::Hash_Remove( pcLB );

		return TRUE;
	}

	static inline BOOL RemObject( cLandBlock *pcLB, cObject *pcObject )
	{
		iterObject_lst itObject = std::find( pcLB->m_lstObjects.begin( ), pcLB->m_lstObjects.end( ), pcObject );

		if ( itObject == pcLB->m_lstObjects.end( ) )
			return FALSE;

		pcLB->m_lstObjects.erase( itObject );
		return TRUE;
	}

	static void		ReceiveAllObjectsInFocus	( cClient *pcClient, cLandBlock *pcLB );
	static void		ReceiveAllObjectsInLandBlock( cClient *pcClient, cLandBlock *pcLB );
	
	static void		LandBlocksWithin( BYTE bDistance, WORD wCenterLB, WORD *pwStore );

	static DWORD	m_dwGUIDCycle_Avatar;
	static DWORD	m_dwGUIDCycle_Object;
	static std::list< DWORD >	m_lstUnusedAvatarGUIDs;
	static std::list< DWORD >	m_lstUnusedObjectGUIDs;

	static DWORD	m_dwConfirmSequence;
	static std::vector< ConfirmPanel >	m_lstConfirmsPending;
};

#endif	// #ifndef __WORLDMANAGER_H