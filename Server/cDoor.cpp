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
 *	@file cDoor.cpp
 *	Handles the door object functions.
 *
 *	This class is referenced whenever a door is created, used, or assessed.
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
 *	Handles the creation of doors.
 *
 *	Called whenever a door object should be initialized.
 */
cDoor::cDoor( WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc)
{
	SetLocation( pcLoc );
	m_dwGUID = dwGUID;//cWorldManager::NewGUID_Object( );
	m_strName.assign( szName );
	m_strDescription.assign( szDesc );
	m_dwModel = type;
	m_dwType = 0;
	m_fStatic = TRUE;
	m_dwDoorState = 0x0CL;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of doors in the world.
 *
 *	This function is called whenever a door should be created in the world for a given client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cDoor::CreatePacket()
{
	cMessage cmCreate;
	unsigned char bUnknown[] = {
		0x00, 0x00, // Animation type 0 - 1
		0x3D, 0x00, // Starting Stance 2 - 3
		0x02, 0xF0, 0xB4, 0x02, // Unknown - Flag 0x00000002 in it 4 - 7
		0x0B, 0x00, 0x00, 0x00, // Status 9 - 11
	};

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );
	
	DWORD dwFlags = 0x00019803L;

	cmCreate << dwFlags;
	cmCreate << WORD( 0x0008 );
	cmCreate << WORD( 0x0001 );

	// MASK 0x00010000 unknown Bytes - Starting Animation  
	cmCreate << 0x0CL;

	CopyMemory(&bUnknown[9],&m_dwDoorState,4);
	cmCreate.pasteData(bUnknown,sizeof(bUnknown));
	//cmCreate << 0x003D0000 << 0x02B4F002 << m_dwDoorState; //0x0000000C; // unknownByte
	cmCreate << 0x0L;

	// MASK 0x00008000 -- Location 
	if ( !m_fIsOwned )
		cmCreate.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
	
	// MASK 0x00000002 -- Animation Set
	DWORD dwAnimC = 0x09000000L + m_wAnimConfig;
	cmCreate << dwAnimC;

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000000L + m_wSoundSet;
	cmCreate << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmCreate << 0x3400002BL;//0x00000000L;

	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + m_dwModel;
	cmCreate << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmCreate	<< m_wPositionSequence
				<< m_wNumAnims //wNuminteracts 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	DWORD dwFlags2 = 0x00000030;
	
	cmCreate << dwFlags2 << m_strName.c_str( ) << WORD(m_wModel) << WORD(m_wIcon);//WORD( 0x019C ) << WORD( 0x1317 );;//WORD( 0x0116 ) << WORD( 0x1317 );
				
	DWORD dwObjectFlags1 = 0x80L;	
	cmCreate << dwObjectFlags1;

	DWORD dwObjectFlags2 = 0x1014L;
	cmCreate << dwObjectFlags2 ;

	// Masked against dwFlags2
	// Mask 0x0010 dwUnknown_v2
	cmCreate << DWORD(0x20);
	// Mask 0x0020 - unknown_v3
	cmCreate << float(2.0);

	return cmCreate;
}

/**
 *	Handles the actions of door objects.
 *
 *	This function is called whenever a door is used or should perform an action.
 */
void cDoor::Action(cClient* who)
{
	cMessage cmUseDoor,cmRet;
	WORD m_wStance;

	if(m_wState == 0)
	{
		m_dwDoorState = 0x0CL;
		m_wStance = 0x3D; // Starting Stance
		m_wState = 1;
	}
	else
	{
		m_dwDoorState = 0x0BL;
		m_wStance = 0x3D;
		m_wState = 0;
	}

	cmUseDoor	<< 0xF74CL					// Packet type
				<< m_dwGUID					// Door's GUID
				<< m_wNumLogins				// number of logins
				<< ++m_wPositionSequence	// sequence number
				<< ++m_wNumAnims			// Number Animations this login session
				<< WORD(0x00)				// Activity
				<< WORD(0x00)
				<< WORD(0x3D)				// Starting Stance
				<< DWORD(0x02B4F002)		// Supposed to be Animation Mask
				<< m_dwDoorState;			// Door state Closed = 0x0C Open = 0x0B

	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmUseDoor, 3 );

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of door objects.
 *
 *	This function is called whenever a door is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cDoor::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x00400000L << 1L << (m_dwType ? "Use this Door to Close" : "Use this Door to Open");
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);

}