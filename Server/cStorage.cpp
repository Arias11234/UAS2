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
 *	@file cStorage.cpp
 *	Implements functionality for housing storage.
 *
 *	This class is referenced whenever a housing storage object is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "Client.h"
#include "MasterServer.h"
#include "Object.h"
#include "WorldManager.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of housing storage chests.
 *
 *	Called whenever a housing storage chest object should be initialized.
 */
cStorage::cStorage( WORD type, DWORD dwGUID, DWORD dwHouseID, char *szName, char *szDescription, cLocation *pcLoc )
{
	SetLocation( pcLoc );
	m_dwGUID = dwGUID;
	m_strName.assign( szName );
	m_strDescription.assign( szDescription );
	m_dwModel = type;
	m_fStatic = TRUE;
	m_dwDoorState = 0x0CL;
	m_dwHouseID = dwHouseID;
	m_dwOwnerID = 0x00000000; // TODO:  Link with housing
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of housing storage chests in the world.
 *
 *	This function is called whenever a housing storage chest object should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cStorage::CreatePacket()
{
	cMessage cmCreate;
	
	char	szCommand[200];
	RETCODE	retcode;
	
	char	OwnerIDBuff[9];
	DWORD	OwnerID = NULL;

	sprintf( szCommand, "SELECT OwnerID FROM houses WHERE wModel = %d;",m_dwHouseID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	// Return SQL_SUCCESS if there is a house that corresponds to the hook's HouseID
	if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) {
		sscanf(OwnerIDBuff,"%08x",&OwnerID);
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

//	float	flpScale;				// Scale used in Packet being Sent
	float	flmScale = 0;			// Model Data Scale
	DWORD	dwFlags1 = 0x00019803;

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );

	cmCreate << dwFlags1;

	cmCreate << WORD(0x0418);	// Type of portal 
	cmCreate << WORD(0x0000);	// Unknown

	//Flags1 Mask: 0x00019803
	{
		//Flags1 & 0x00010000 -- DWORD byteCount
		DWORD dwUnkCount = 0x0000000C;
		BYTE unknownByte[0x0C] = {	0x00,0x00,0x3D,0x00,0x02,0x00,0x00,0x00,
									0x0C, // model appearance:  owned = 0x0B; unowned = 0x0A, 0x0C?
									0x00,0x00,0x00};

		cmCreate << dwUnkCount;
			
		for ( int i = 0; i < dwUnkCount; ++i ) {
			cmCreate << unknownByte[i];
		}
		DWORD dwUnknownDword = 0x0L;
		cmCreate << dwUnknownDword;

		//Flags1 & 0x00008000 -- Location 
		cmCreate.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );	// Next comes the location
		
		//Flags1 & 0x0000002 -- DWORD ResourceID Animations
		DWORD dwAnimC = 0x09000000L + m_wAnimConfig;
		cmCreate << dwAnimC;
		
		//Flags1 & 0x0000800 -- DWORD ResourceID Sounds
		DWORD dwSoundSet = 0x20000000L + m_wSoundSet;
		cmCreate << dwSoundSet;
		
		//Flags1 & 0x00001000 -- DWORD ResourceID
		cmCreate << 0x3400002B; // unknown ResourceID
		
		//Flags1 & 0x00000001 -- DWORD ResourceID dwModel = 0x02000001; // the model.
		DWORD dwModel = 0x02000000L + m_dwModel;
		cmCreate << dwModel;
	}

	// SeaGreens
	WORD wNuminteracts = 0x0004;
	WORD wNumbubbles = 0x0000;
	WORD wNumJumps = 0x0000;
	WORD wUnkFlag6 = 0x0000;	
	WORD wNumOverrides = 0x0040;
	WORD wUnkFlag8 = 0x0000;
	WORD wUnkFlag10 = 0x0000;

	cmCreate	<< m_wPositionSequence	// movement 0x0001
				<< wNuminteracts		// animations
				<< wNumbubbles			// bubble modes
				<< wNumJumps			// num jumps
				<< m_wNumPortals 
				<< wUnkFlag6			// anim count
				<< wNumOverrides		// overrides
				<< wUnkFlag8
				<< WORD(0x7A51)			// m_wNumLogins // 0x0D4B
				<< wUnkFlag10;
	
	DWORD dwFlags2 = 0x0220003E;		// Flags2 -- Defines what data comes next

	cmCreate << dwFlags2;
	
	cmCreate << Name( );	// Object's Name
	
	cmCreate << WORD(m_wModel) << WORD(m_wIcon);

	DWORD dwObjectFlags1 = 0x00000200; // 0x0200 Containers
	DWORD dwObjectFlags2 = 0x00000015; // 0x0010 is not an npc; 0x0004 cannot be picked up; 0x0001 can be opened (false if locked) 

	cmCreate << dwObjectFlags1 << dwObjectFlags2;

	//Flags2 Mask: 0x0220003E
	{
		// Flags2 & 0x00000002 -- BYTE itemSlots
		cmCreate << BYTE(0x1A);		// number of item slots

		// Flags2 & 0x00000004 -- BYTE packSlots
		cmCreate << BYTE(0x01);		// number of pack slots (a pack slot is a slot that may hold a pack or a foci)

		// Flags2 & 0x00000008 -- DWORD value
		cmCreate << 0x000000C8;		// object value

		// Flags2 & 0x00000010 -- DWORD unknown10
		cmCreate << 0x00000030;

		// Flags2 & 0x00000020 -- float approachDistance
		float flApproach = 1.0f;	// 0x3F800000
		cmCreate << flApproach;		// distance a player will walk to use the object

		// Flags2 & 0x00200000 -- WORD burden
		cmCreate << WORD(0xBBAF);	// total burden of this object

		// Flags2 & 0x02000000 -- ObjectID owner
		cmCreate << OwnerID;		// the owner of this object
	}

	return cmCreate;
}

/**
 *	Handles the actions of housing storage chest objects.
 *
 *	This function is called whenever a housing storage chest is used or should perform an action.
 */
void cStorage::Action(cClient* who)
{
	cMessage cmUseChest;

	WORD m_wStance;

	if(m_wState == 0)  //CLOSED
	{
		m_dwDoorState = 0x0CL;
		m_wStance = 0x3D; // Starting Stance
		m_wState = 1;
	}
	else //OPEN
	{
		m_dwDoorState = 0x0BL;
		m_wStance = 0x3D;
		m_wState = 0;
	}
	
	//Play Animation
	cmUseChest	<< 0xF74CL					// Packet type
				<< m_dwGUID					// Door's GUID
				<< m_wNumLogins				// number of logins
				<< ++m_wPositionSequence	// sequence number
				<< ++m_wNumAnims			// Number Animations this login session
				<< WORD(0x00)				// Activity
				<< WORD(0x00)
				<< WORD(0x3D)				// Starting Stance
				<< DWORD(0x00000002)		// Supposed to be Animation Mask
				<< m_dwDoorState;			// Door state Closed = 0x0C Open = 0x0B
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmUseChest, 3 );

	if(m_wState == 0)
	{
		//Display the item slots in the chest (0x0196)
		cMessage cmSetPackContents;
		cmSetPackContents << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0196L << m_dwGUID;
		who->AddPacket(WORLD_SERVER,cmSetPackContents,4);
		
		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
	}

	if(m_wState == 1)
	{
		//Close Container
		cMessage cmCloseContainer;
		cmCloseContainer << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x0052L << m_dwGUID;
		who->AddPacket(WORLD_SERVER,cmCloseContainer,4);

		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
		who->AddPacket(WORLD_SERVER,cmActionComplete,4);
	}
}

/**
 *	Handles the assessment of housing storage chest objects.
 *
 *	This function is called whenever a housing storage chest is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cStorage::Assess(cClient *pcAssesser)
{
//	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );
	cMessage cmAssess;
	DWORD flags = 0x0000000B;
	m_strDescription.assign("Use this item to open it and see its content");

	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags
	<< 0x01L					// Success = 0x01; Failure = 0x00

	// Flags & 0x00000001
	<< WORD(0x0002)				// Total number of DWORDS
	<< WORD(0x0010)				// Unknown
		<< 0x13L				// Value
		<< 0xC8L				// DWORD value
		<< 0x05L				// Burden
		<< 0x0001BBAFL			// DWORD value

	// Flags & 0x00000002
	<< WORD(0x0001)				// Total number of DWORDS
	<< WORD(0x0008)				// Unknown	
		<< 0x02L				// Value
		<< 0x00L				// DWORD value

	// Flags & 0x00000008
	<< WORD(0x0002)				// Total number of DWORDS
	<< WORD(0x0008)				// Unknown	
		<< 0x10L;				// DWORD Full Description
		cmAssess << "Owned by Cubem0j0"
		<< 0x0EL				// DWORD Usage Instructions
		<< this->m_strDescription.c_str();

	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}