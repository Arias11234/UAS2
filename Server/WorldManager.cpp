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
 *	@file WorldManager.cpp
 *	Encapsulates basic world functionality.  Maintains client and object lists.
 *	Loads world objects and populates/depopulates landblock object lists.
 *	Also controls when and which objects' Create Packets are sent to the client.
 *		@see ReceiveAllObjectsInLandBlock() - encapsulates the functionality to create the objects within a given avatar's landblock.
 *		@see ReceiveAllObjectsInFocus() - encapsulates funtionality to create tthe objects with the area of a given avatar's landblock.
 *		@see LandBlocksWithin() - determines which landblocks are in the area of a given avatar's landblock. 
 */

//TODO: When a landblock is disabled, the important information should be saved to disk. Important
//		information being things like corpses.

//Your location is Landblock: a5b4002b, X: 133.4, Y: 51.5, Z: 52.0, H: 184.8  
//Your location is Landblock: a5b30030, X: 127.6, Y: 169.9, Z: 55.9, H: 184.8

#define RADAR_RADIUS	120

#include "MasterServer.h"
#include "WorldManager.h"
#include "Client.h"
#include "cModels.h"
#include "cMagicModels.h"

//Initialize static members
//=============================================================
DWORD	cWorldManager::m_dwGUIDCycle_Avatar;
DWORD	cWorldManager::m_dwGUIDCycle_Object;
std::list< DWORD >		cWorldManager::m_lstUnusedAvatarGUIDs;
std::list< DWORD >		cWorldManager::m_lstUnusedObjectGUIDs;

std::vector<ConfirmPanel>	cWorldManager::m_lstConfirmsPending;
DWORD	cWorldManager::m_dwConfirmSequence;

char	cWorldManager::g_szAccessFile[MAX_PATH+20];
cLandBlock *cLandBlock::m_lpcHashTable[510];
WORD	wTmpMoveCounter;
//=============================================================

/**
 *	Loads the initial world objects
 *
 *	This function is called when the server is started.
 */
void cWorldManager::Load( )
{
	cDatabase::InitializeAvatarGUIDs( m_dwGUIDCycle_Avatar );
	cDatabase::InitializeObjectGUIDs( m_dwGUIDCycle_Object );
	cLandBlock::Hash_Load( );
	cWorldManager::m_dwConfirmSequence	= 0x00000000;
	
//	UpdateConsole( " --------------------------------------------------------------------------\r\n\r\n" );
	UpdateConsole( " Starting to load the world:\r\n\r\n" );

	cDatabase::LoadModelList( );
//		UpdateConsole( "\r\n Model list has been loaded.\r\n\r\n");

	cMasterServer::LoadItemModels( );
		UpdateConsole( "\r\n Item model loading complete.\r\n\r\n" );

	cMasterServer::LoadWorldModels( );
		UpdateConsole( "\r\n World model loading complete.\r\n\r\n" );

	cMasterServer::LoadSpellModels( );
		UpdateConsole( "\r\n Spell model loading complete.\r\n\r\n" );

	cMasterServer::LoadSpells( );
		UpdateConsole( "\r\n Spell loading complete.\r\n\r\n" );
	
	cMasterServer::LoadSpellComps( );
		UpdateConsole( "\r\n Spell component loading complete.\r\n\r\n" );

	cMasterServer::LoadWorldObjects( );
		UpdateConsole( "\r\n World objects loading complete.\r\n\r\n" );

	cMasterServer::LoadWorldObjects2( );
		UpdateConsole( "\r\n Merchant sign loading complete.\r\n\r\n" );

	cMasterServer::LoadHousing( );
		UpdateConsole( "\r\n Housing object loading complete.\r\n\r\n" );

	cMasterServer::LoadCovenants( );
		UpdateConsole( "\r\n Covenant crystal loading complete.\r\n\r\n" );
				
	cMasterServer::LoadHooks( );
		UpdateConsole( "\r\n Housing hooks loading complete.\r\n\r\n" );

	cMasterServer::LoadStorage( );
		UpdateConsole( "\r\n Housing storage loading complete.\r\n\r\n" );

	cMasterServer::LoadDoors( );
		UpdateConsole( "\r\n Door loading completed.\r\n\r\n" );

	cMasterServer::LoadChests( );
		UpdateConsole( "\r\n Chest loading complete.\r\n\r\n" );

	cMasterServer::LoadLifestones( );
		UpdateConsole( "\r\n Lifestone loading complete.\r\n\r\n" );

	cMasterServer::LoadPortals( );
		UpdateConsole( "\r\n Portal loading complete.\r\n\r\n" );

	cMasterServer::LoadAltars( );
		UpdateConsole( "\r\n Altar loading complete.\r\n\r\n" );

	cMasterServer::LoadNPCModels( );
		UpdateConsole( "\r\n NPC model loading complete.\r\n\r\n" );

  	cMasterServer::LoadMonsterModels( );
		UpdateConsole( "\r\n Monster model loading complete.\r\n\r\n" );

	cMasterServer::LoadNPCs( );
		UpdateConsole( "\r\n NPC loading complete.\r\n\r\n" );

	cMasterServer::LoadGroundSpawns( );
		UpdateConsole( "\r\n Ground spawn loading complete.\r\n\r\n" );

	cMasterServer::LoadMonsters( );
		UpdateConsole( "\r\n Monster loading complete.\r\n\r\n" );

	UpdateConsole( " World loading complete!\r\n\r\n" );
}

void cWorldManager::Unload( )
{
	cLandBlock::Hash_Unload( );
}

/**
 *	Handles the processing for when an avatar moves.
 *
 *	This function is called whenever an avatar moves.  The function checks whether the avatar
 *	has moved into a new landblock and populates/depopulates landblocks appropriately.
 *	The function also sends the movement animation to all clients within focus.
 *
 *	@param *pcClient - A pointer to the client whose avatar is being teleported.
 *	@param &NewLoc - The address of the cLocation struct value that represents the location to which the avatar is being teleported.
 *	@param wAnim - The numeric value of the animation to play.
 *	@param flPlay - The speed at which to play the gievn animation.
 *
 *	@return BOOL - A boolean value representing whether the avatar's landblock has not changed.
 */
BOOL cWorldManager::MoveAvatar( cClient *pcClient, cLocation& NewLoc, WORD wAnim, float flPlaySpeed )
{
	WORD wOldLB			= HIWORD( pcClient->m_pcAvatar->m_Location.m_dwLandBlock );
	WORD wNewLB			= HIWORD( NewLoc.m_dwLandBlock );
  	WORD wO_LB			= LOWORD( pcClient->m_pcAvatar->m_Location.m_dwLandBlock );
	WORD wN_LB			= LOWORD( NewLoc.m_dwLandBlock );
	cLandBlock *pcNewLB = cLandBlock::Hash_New( wNewLB );
	SimpleAI::ExecuteActions( );
	
	/*
	#ifdef _DEBUG
		char szPacketA[120];
		sprintf( szPacketA, "Move: LB: %08x X: %f Y: %f Z: %f H1: %f U1: %f U2: %f H2: %f",NewLoc.m_dwLandBlock,NewLoc.m_flX, NewLoc.m_flY,NewLoc.m_flZ,NewLoc.m_flA,NewLoc.m_flB, NewLoc.m_flC,NewLoc.m_flW);
		cMasterServer::ServerMessage( ColorBlue,pcClient,(char *)szPacketA);
	 #endif
	*/
	cLocation OldLoc = pcClient->m_pcAvatar->m_Location;
	pcClient->m_pcAvatar->SetLocation( NewLoc );

 	if ( wOldLB != wNewLB )
	{
		cLandBlock *pcOldLB = cLandBlock::Hash_Find( wOldLB );
		if ( !pcOldLB )
			return FALSE;

		WORD wLBUnFocus[9];
		WORD wLBFocus[9];
		cMessage cmLogin;
		// Must be signed to allow for negatives.
		short sDiffX		= HIBYTE( wNewLB ) - HIBYTE( wOldLB );
		short sDiffY		= LOBYTE( wNewLB ) - LOBYTE( wOldLB );
		cMessage cmCreate	= pcClient->m_pcAvatar->CreatePacket( );
		BYTE bChangeSize	= 0;
		
		*((DWORD *)&MSG_00000002[4]) = pcClient->m_pcAvatar->GetGUID( );
		cmLogin.CannedData( MSG_00000002, sizeof( MSG_00000002 ) );

		if ( sDiffY )
		{
			wLBFocus[0] = wNewLB + sDiffY * 2;
			wLBFocus[1] = wLBFocus[0] - 0x0100;
			wLBFocus[2] = wLBFocus[0] + 0x0100;
			wLBFocus[3] = wLBFocus[0] - 0x0200;
			wLBFocus[4] = wLBFocus[0] + 0x0200;

			wLBUnFocus[0] = wOldLB - sDiffY * 2;
			wLBUnFocus[1] = wLBUnFocus[0] - 0x0100;
			wLBUnFocus[2] = wLBUnFocus[0] + 0x0100;
			wLBUnFocus[3] = wLBUnFocus[0] - 0x0200;
			wLBUnFocus[4] = wLBUnFocus[0] + 0x0200;

			bChangeSize = 5;
		}

		if ( sDiffX )
		{
			if ( !bChangeSize )
			{
				wLBFocus[0] = wNewLB + sDiffX * 0x0200;
				wLBFocus[1] = wLBFocus[0] - 1;
				wLBFocus[2] = wLBFocus[0] + 1;
				wLBFocus[3] = wLBFocus[0] - 2;
				wLBFocus[4] = wLBFocus[0] + 2; 

				wLBUnFocus[0] = wOldLB - sDiffX * 0x0200;
				wLBUnFocus[1] = wLBUnFocus[0] - 1;
				wLBUnFocus[2] = wLBUnFocus[0] + 1;
				wLBUnFocus[3] = wLBUnFocus[0] - 2;
				wLBUnFocus[4] = wLBUnFocus[0] + 2;

				bChangeSize	= 5;
			}
			else
			{
				wLBFocus[5] = wNewLB + sDiffX * 0x0200;
				wLBFocus[6] = wLBFocus[5] - 1;
				wLBFocus[7] = wLBFocus[5] + 1;

				wLBUnFocus[5] = wOldLB - sDiffX * 0x0200;
				wLBUnFocus[6] = wLBUnFocus[5] - 1;
				wLBUnFocus[7] = wLBUnFocus[5] + 1;

				if ( sDiffY > 0 )
				{
					wLBFocus[8]		= wLBFocus[5] - 2;
					wLBUnFocus[8]	= wLBUnFocus[5] + 2;
				}
				else
				{
					wLBFocus[8]		= wLBFocus[5] + 2;
					wLBUnFocus[8]	= wLBUnFocus[5] - 2;
				}

				bChangeSize	= 9;
			}
		}

		cLandBlock *pcRemFocus; 
		cLandBlock *pcAddFocus;
		for ( int i = 0; i < bChangeSize; ++i )
		{
			pcRemFocus = cLandBlock::Hash_Find( wLBUnFocus[i] );
			pcAddFocus = cLandBlock::Hash_New( wLBFocus[i] );

			AddFocus( pcAddFocus, pcClient );

			if ( pcRemFocus )
				RemFocus( pcRemFocus, pcClient );

			//ReceiveAllObjectsInLandBlock( pcClient, pcAddFocus );

			ReceiveAllObjectsInFocus( pcClient, pcAddFocus );
			SendToAllInLandBlock( pcAddFocus, cmCreate, 3 );
			SendToAllInLandBlock( pcAddFocus, cmLogin, 3 );
		}

		// Guaranteed to happen for any kind of movement.
		
		AddClient( pcNewLB, pcClient );
		AddFocus( pcOldLB, pcClient );
		RemFocus( pcNewLB, pcClient );
		RemClient( pcOldLB, pcClient );
	}
//	 	if (( wOldLB == wNewLB ) && (HIBYTE(wO_LB) != HIBYTE( wN_LB)))
	 	if (( wOldLB == wNewLB ) && (wTmpMoveCounter > 60))
		{
 			ReceiveAllObjectsInFocus( pcClient, pcNewLB );

			wTmpMoveCounter = 0;
		}

	if ( wAnim )
	SendToOthersInFocus( pcNewLB, pcClient, pcClient->m_pcAvatar->Animation( wAnim, flPlaySpeed ), 3 );
	
	SendToAllInFocus( pcNewLB, pcClient->m_pcAvatar->LocationPacket( ), 3 );

	wTmpMoveCounter = wTmpMoveCounter + 1 ;

	if ( cObject *CollisionPortal = GetCollisionPortal( OldLoc, NewLoc ) )
		CollisionPortal->Action( pcClient );

	return TRUE;
}

/**
 *	Handles the processing for when an avatar is teleported
 *
 *	This function is called whenever an avatar is teleported.
 *
 *	@param *pcClient - A pointer to the client whose avatar is being teleported.
 *	@param &NewLoc - The address of the cLocation struct value that represents the location to which the avatar is being teleported.
 *
 *	@return BOOL - A boolean value representing whether the avatar's landblock has not changed.
 */
BOOL cWorldManager::TeleportAvatar( cClient *pcClient, cLocation& NewLoc )
{
	WORD wOldLB			= HIWORD( pcClient->m_pcAvatar->m_Location.m_dwLandBlock );
	cLandBlock *pcOldLB	= cLandBlock::Hash_Find( wOldLB );
	cLocation OldLoc	= pcClient->m_pcAvatar->m_Location;

	if ( !pcOldLB )
		return FALSE;

	pcClient->m_pcAvatar->m_wPortalCount += 2;
	pcClient->m_pcAvatar->SetLocation( NewLoc );

	SendToAllInFocus( pcOldLB, pcClient->m_pcAvatar->LocationPacket( ), 3 );

	pcClient->m_pcAvatar->SetLocation( OldLoc );

	if ( !RemoveClient( pcClient, FALSE, FALSE ) )
		return FALSE;

	pcClient->m_pcAvatar->SetLocation( NewLoc );

	return TRUE;
}

/**
 *	Handles the processing for when a client is removed from the server.
 *
 *	This function is called whenever an avatar removed from the server
 *
 *	@param *pcClient - A pointer to the client whose avatar is being removed.
 *	@param fRemoveAvatar - A value representing whether the avatar should be removed.
 *	@param fDeleteavatar - A value representing whether the avatar object should be deleted from working memory.
 *
 *	@return BOOL - A boolean value representing whether the avatar is located in a known landblock.
 */
BOOL cWorldManager::RemoveClient( cClient *pcClient, BOOL fRemoveAvatar, BOOL fDeleteAvatar )
{
	WORD wLBs[25];
	WORD wLandBlock		= HIWORD( pcClient->m_pcAvatar->m_Location.m_dwLandBlock );
	cLandBlock *pcLB	= cLandBlock::Hash_Find( wLandBlock );

	if ( !pcLB )
		return FALSE;
	
	if ( pcClient->m_pcAvatar )
	{
		if ( fRemoveAvatar )
		{
			char szIP[32];
			cMasterServer::FormatIP_Port( pcClient->m_saSockAddr, szIP );
			UpdateConsole(	" World Server disconnection:\r\n"
							"      User: %s\r\n"
							"      Avatar: %s\r\n"
							"      IP: %s\r\n", pcClient->m_szAccountName, pcClient->m_pcAvatar->m_strName.c_str(), szIP);
			
			RemoveAvatar( pcClient, pcLB, 0x011B );
			//Karki
			cMasterServer::m_UserCount--;
			cMasterServer::ServerMessage( ColorMagenta, NULL, "%s has left %s. %d client(s) connected.", pcClient->m_pcAvatar->Name( ), cMasterServer::m_szServerName, cMasterServer::m_UserCount);
			//EndKarki
		}
		if ( fDeleteAvatar )
			SAFEDELETE( pcClient->m_pcAvatar )
	}

	RemClient( pcLB, pcClient );

	LandBlocksWithin( 2, wLandBlock, wLBs );
	for ( int i = 0; i < 24; ++i )
	{
		pcLB = cLandBlock::Hash_Find( wLBs[i] );
		if ( pcLB )
			RemFocus( pcLB, pcClient );
	}

	return TRUE;
}

/**
 *	Handles the processing for when a client is added to the server.
 *
 *	This function is called whenever an avatar is added to the server.
 *
 *	@param *pcClient - A pointer to the client whose avatar is being added.
 *	@param fSpawnAvatar - A value representing whether the avatar should be spawned.
 */
BOOL cWorldManager::AddClient( cClient *pcClient, BOOL fSpawnAvatar )
{
	WORD wLBs[25];
	WORD wLandBlock		= HIWORD( pcClient->m_pcAvatar->m_Location.m_dwLandBlock );
	cLandBlock *pcLB	= cLandBlock::Hash_New( wLandBlock );

	AddClient( pcLB, pcClient );

	LandBlocksWithin( 2, wLandBlock, wLBs );
	cLandBlock *pcFLB;
	for ( int i = 0; i < 24; ++i )
	{
		pcFLB = cLandBlock::Hash_New( wLBs[i] );
		AddFocus( pcFLB, pcClient );
	}

	if ( fSpawnAvatar )
		SpawnAvatar( pcClient, pcLB );

	return TRUE;
}

/**
 *	Handles the processing of objects within an avatar's landblock.
 *
 *	Sends the client the create messages of all objects within the specified landblock.
 *	This is accomplished by iterating through all the clients within the landblock and sending
 *	their Create Packets (excluding that packet for the client receiving them) and then 
 *	iterating through all the other objects and sending their Create Packets.
 *
 *	@param *pcClient - A pointer to the client whose avatar should receive the packets.
 *	@param *pcLB - The landblock in which the given avatar resides.
 */
void cWorldManager::ReceiveAllObjectsInLandBlock( cClient *pcClient, cLandBlock *pcLB )
{
	//Iterate through all the clients within the landblock
	for ( iterClient_lst itClient = pcLB->m_lstClients.begin( ); itClient != pcLB->m_lstClients.end( ); ++itClient )
	{
		//Exclude the pcClient
		if ( pcClient != *itClient )
		{
			cMessage cmLogin;

			*((DWORD *)&MSG_00000002[4]) = (*itClient)->m_pcAvatar->GetGUID( );
			cmLogin.CannedData( MSG_00000002, sizeof( MSG_00000002 ) );
		
			/*BOOL fContinue;
			do
			{
				cMessage cmChildren = ( *itClient )->m_pcAvatar->CreateChildren( fContinue );
				if( fContinue )
					pcClient->AddPacket( WORLD_SERVER, cmChildren, 3 );
			} while ( fContinue );*/

			( *itClient )->m_pcAvatar->CreateChildren( pcClient );

			pcClient->AddPacket( WORLD_SERVER, (*itClient)->m_pcAvatar->CreatePacket( ), 3 );
			pcClient->AddPacket( WORLD_SERVER, cmLogin, 3 );
		}
	}

	//Iterate through all the objects within the landblock
	for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); ++itObject )
	{
		pcClient->AddPacket( WORLD_SERVER, (*itObject)->CreatePacket( ), 3 );
	}
	
}

/**
 *	Sends the client the Create Messages for all of the objects within the focus range of the specified landblock.
 *
 *	Iterates through the landblocks within a specified range of the avatar's current landblock.
 *	A call is made to LandBlocksWithin() to determine which landblocks lie within the specified range.
 *	For each landblock with the range, a call to ReceiveAllObjectsInLandBlock() is made.
 *
 *	@param *pcClient - A pointer to the client whose avatar should referenced.
 *	@param *pcLB - The landblock in which the given avatar resides.
 */
void cWorldManager::ReceiveAllObjectsInFocus( cClient *pcClient, cLandBlock *pcLB )
{
	WORD wLBs[25];

	LandBlocksWithin( 2, pcLB->m_wLandBlock, wLBs );

	// Receive all the objects in the landblock itself first.
	ReceiveAllObjectsInLandBlock( pcClient, pcLB );
	for ( int i = 0; i < 24; ++i )
	{
		pcLB = cLandBlock::Hash_Find( wLBs[i] );

		if ( pcLB )
		{
			//cMasterServer::LoadSpawns(pcLB->m_wLandBlock);
			ReceiveAllObjectsInLandBlock( pcClient, pcLB );
		}
		
	}
}

/**
 *	Adds an object to object list of the landblock in which it is located.
 *
 *	@param *pcObject - A pointer to the object to add.
 *	@param fSpawnObject - A value representing whether the object should be spawned.
 */
BOOL cWorldManager::AddObject( cObject *pcObject, BOOL fSpawnObject )
{
	//Find the object's landblock
	WORD wLandBlock		= HIWORD( pcObject->m_Location.m_dwLandBlock );
	cLandBlock *pcLB	= cLandBlock::Hash_Find( wLandBlock );

	if ( !pcLB )
		pcLB = cLandBlock::Hash_New( wLandBlock );

	AddObject( pcLB, pcObject );

	if ( fSpawnObject )
		SpawnObject( pcObject, pcLB );

	return TRUE;
}
/**
 *	Removes an object to object list of the landblock in which it is located.
 *
 *	@param *pcObject - A pointer to the object to remove.
 *	@param fRemoveObject - A value representing whether the object should be removed.
 *	@param fDeleteObject - A value representing whether the object should be deleted from working memory.
 */
BOOL cWorldManager::RemoveObject( cObject *pcObject, BOOL fRemoveObject, BOOL fDeleteObject )
{
	WORD wLandBlock		= HIWORD( pcObject->m_Location.m_dwLandBlock );
	cLandBlock *pcLB	= cLandBlock::Hash_Find( wLandBlock );

	if ( !pcLB )
		return FALSE;

	RemObject( pcLB, pcObject );

	if ( fRemoveObject )
		RemoveObject( pcObject, pcLB );
	if ( fDeleteObject )
		SAFEDELETE( pcObject )

	return TRUE;
}

//// Object changes Landblock location
/**
 *	Adds an object to object list of the landblock in which it is located.
 *
 *	Used for when an object changes landblock location.
 *
 *	@param *pcObject - A pointer to the object to add.
 */
BOOL cWorldManager::MoveAddObject( cObject *pcObject )
{
	WORD wLandBlock		= HIWORD( pcObject->m_Location.m_dwLandBlock );
	cLandBlock *pcLB	= cLandBlock::Hash_Find( wLandBlock );

	if ( !pcLB )
		pcLB = cLandBlock::Hash_New( wLandBlock );

	AddObject( pcLB, pcObject );

	return TRUE;
}
/**
 *	Removes an object from the object list of the landblock in which it is located.
 *
 *	Used for when an object changes landblock location.
 *
 *	@param *pcObject - A pointer to the object to remove.
 *
 *	@return BOOL - A boolean value representing whether the object is located in a known landblock.
 */
BOOL cWorldManager::MoveRemObject( cObject *pcObject )
{
	WORD wLandBlock		= HIWORD( pcObject->m_Location.m_dwLandBlock );
	cLandBlock *pcLB	= cLandBlock::Hash_Find( wLandBlock );

	if ( !pcLB )
		return FALSE;

	RemObject( pcLB, pcObject );

	return TRUE;
}
//////////////////////////////////////////////////////

/**
 *	Removes all object from the object list of all the landblocks.
 *
 *	This is accomplished by iterating through each landblock, for each landblock iterating through each object, 
 *	and deleting the objects as they are iterated through.
 */
void cWorldManager::RemoveAllObjects()
{
	for ( int i = 0; i < 510; ++i )
	{
		cLandBlock *pcLB = cLandBlock::m_lpcHashTable[i];
		while ( pcLB )
		{
			 for ( iterObject_lst itObject = pcLB->m_lstObjects.begin( ); itObject != pcLB->m_lstObjects.end( ); )
			 {
				 if( !(*itObject)->IsStatic( ) )
				 {
					RemoveObject( (*itObject), pcLB );
					SAFEDELETE( *itObject );
					itObject = pcLB->m_lstObjects.erase( itObject );
				 }
				 else
					++itObject;
			 }
			pcLB = pcLB->m_pcNext;
		}
	}
}

/**
 *	Takes a center landblock and returns the landblocks around it.
 */
void cWorldManager::LandBlocksWithin( BYTE bDistance, WORD wCenterLB, WORD *pwStore )
{
	static WORD i = 0;
	WORD wDiameter = 2 * bDistance + 1;

	pwStore[i++] = wCenterLB + bDistance + (0x0100 * bDistance);

	int j = 1, k = -1;
	while ( 1 )
	{
		if ( j++ >= wDiameter )
		{
			if ( k == -1 )
			{
				k = -0x0100;
				j = 1;
				continue;
			}
			else if ( k == -0x0100 )
			{
				k = 1;
				j = 1;
				continue;
			}
			else if ( k == 1 )
			{
				k = 0x0100;
				j = 2;
				continue;
			}
			else
				break;
		}

		pwStore[i++] = pwStore[i-1] + k;
	}

	if ( bDistance > 1 )
		LandBlocksWithin( --bDistance, wCenterLB, pwStore );
	else
	{
		pwStore[i] = wCenterLB;
		i = 0;
	}
}

void cWorldManager::AddPendingConfirm(DWORD sequence, DWORD type, std::string szText, DWORD senderGUID, DWORD receiptGUID)
{
	ConfirmPanel aConfirmPanel;

	aConfirmPanel.m_dwSequence = sequence;
	aConfirmPanel.m_dwType = type;
	aConfirmPanel.m_szText = szText;

	aConfirmPanel.m_dwSenderGUID = senderGUID;
	aConfirmPanel.m_dwReceiptGUID = receiptGUID;

	m_lstConfirmsPending.push_back(aConfirmPanel);
}

void cWorldManager::FindPendingConfirm( DWORD dwSeq, DWORD dwReply )
{
	for( cWorldManager::iterConfirmPanel_lst itConfirmPanel = m_lstConfirmsPending.begin(); itConfirmPanel != m_lstConfirmsPending.end(); ++itConfirmPanel )
	{
		if ( itConfirmPanel->m_dwSequence == dwSeq )
		{
			cClient *pcSendClient = cClient::FindClient( itConfirmPanel->m_dwSenderGUID );
			cClient *pcRecvClient = cClient::FindClient( itConfirmPanel->m_dwReceiptGUID );			
				
			switch (itConfirmPanel->m_dwType)
			{
				case 1:
					{
						pcRecvClient->m_pcAvatar->SwearAllegiance(itConfirmPanel->m_szText, dwReply, itConfirmPanel->m_dwSenderGUID);
						break;
					}
				case 2:
					{
						break;
					}
				case 3:
					{
						break;
					}					
				case 4:
					{
						pcRecvClient->m_pcAvatar->FellowshipRecruitRecv(itConfirmPanel->m_szText, dwReply, itConfirmPanel->m_dwReceiptGUID);
						pcSendClient->m_pcAvatar->FellowshipRecruitSend(itConfirmPanel->m_szText, dwReply, itConfirmPanel->m_dwSenderGUID);
						break;
					}
				case 5:
					{
						break;
					}
			}

			m_lstConfirmsPending.erase(itConfirmPanel);
			break;	// the "itConfirmPanel" iterator is now invalid; must break
		}
	}
}