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
 *	@file cPortal.cpp
 *	Implements functionality for portals.
 *
 *	This class is referenced whenever a portal is created, used, or assessed.
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
 *	Handles the creation of portals.
 *
 *	Called whenever a portal object should be initialized.
 */
cPortal::cPortal( DWORD dwGUID, DWORD dwColor, cLocation *Loc, cLocation *destLoc, char *szName, char *szDescription, DWORD dwLowerRestrict, DWORD dwHigherRestrict )
{
	SetLocation(Loc);
	SetDestination(destLoc);
	m_dwGUID = dwGUID;
	m_strName.assign( szName );
	m_strDescription.assign( szDescription );

	m_dwLowerRestriction = dwLowerRestrict;
	m_dwHigherRestriction = dwHigherRestrict;
	m_fStatic = TRUE;

	m_wPortalMode = 3084; // Will add special case code later, for now all are Auto Use Portals
	// Object Flags appear to be Static
	m_dwObjectsFlag1 = 0x00010080;
	m_dwObjectsFlag2 = 0x00040014; 

	switch(dwColor)
			{
			case 17:
				m_dwModel = 1490; // 0x05D2
				break;
			case 1:
				m_dwModel = 1491;
				break;
			case 4:
				m_dwModel = 1492;
				break;
			case 6:
				m_dwModel = 1493; // 0x05D5
				break;
			case 3:
				m_dwModel = 1494;
				break;
			case 5: 
				m_dwModel = 435;  // 0x01B3
				break;
			case 2:
				m_dwModel = 1780;
				break;
			default:
				m_dwModel = 435;
			}
	m_wModel		=	1032;
	m_wIcon			=	4203;
	m_wAnimConfig	= 0x3;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of portals in the world.
 *
 *	This function is called whenever a portal should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cPortal::CreatePacket()
{
	cMessage cmCreate;
	unsigned char bUnknown[] = {
		0x00, 0x00, // Animation type 0 - 1
		0x3D, 0x00, // Starting Stance 2 - 3
		0x00, 0xF0, 0x99, 0x02, // Unknown - Flag 0x00000002 in it 4 - 7
	};

	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 )  // Special Portals Might have 1
				<< BYTE( 0 ) 
				<< BYTE( 0 );
	//If Portal has a Palette Change will be 0xFF(-255)
	// cmCreate << WORD(0xFF);
	DWORD dwFlags = 0x00018003L; // Appears to be fixed for all Portal Types

	cmCreate << dwFlags;
	cmCreate << WORD( m_wPortalMode ); 
	// This changes (so far only 2 variations occur)
	// 0x3076 - House Portals (Click to Use?)
	// 0x3084 - Normal Portals

	cmCreate << WORD( 0x0000 ); // wUnknown1 Always 0x0000

	// MASK 0x00010000 unknown Bytes - Starting Animation  
	cmCreate << 0x08L;
	cmCreate.pasteData(bUnknown,sizeof(bUnknown));
	cmCreate << 0x0L; // wUnknown_1 Always 0x0000

	// MASK 0x00008000 - Location 
	if ( !m_fIsOwned )
		cmCreate.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
	
	// MASK 0x00000002 Animation Set
	DWORD dwAnimC = 0x09000000L + m_wAnimConfig; 
	// Always 0x9000003 Will still do it this way incase we find a variant
	cmCreate << dwAnimC;

	// MASK 0x00000001 ModelNumber
	DWORD dwModel = 0x02000000L + m_dwModel; // Set on object create based on Database info
	cmCreate << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmCreate	<< WORD(70) // m_wPositionSequence
				<< wNuminteracts
				<< wNumbubbles 
				<< wNumJumps 
				<< WORD(0x0000) // m_wNumPortals 
				<< WORD(0x0001) // m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	DWORD dwFlags2 = 0x00800030; 
	// This changes (so far only 2 variations occur)
	// 0x02800030 for House Portals
	// 0x00800030 for Normal Portals
	
	cmCreate	<< dwFlags2 
				<< m_strName.c_str( ) 
				<< WORD(m_wModel)  // Varies based on Portal Color
				<< WORD(0x106B);   // Icon is static value all portals
				
	//DWORD dwObjectFlags1 = 0x80L;	
	cmCreate << m_dwObjectsFlag1;

	//DWORD dwObjectFlags2 = 0x1014L;
	cmCreate << m_dwObjectsFlag2 ;

	// Masked against dwFlags2
	// Mask 0x0010 dwUnknown_v2 
	// Always 32
	cmCreate << DWORD(0x20);

	// Mask 0x0020 - unknown_v3 Approach Distance
	cmCreate << float(-0.1);
	
	// Mask 0x00800000 - unknown_v6
	// Always 4
	cmCreate << BYTE(0x04);

	this->item_type = 1000;
	
	return cmCreate;
}	

/**
 *	Handles the actions of portal objects.
 *
 *	This function is called whenever a portal is used or should perform an action.
 */
void cPortal::Action(cClient *who)
{
	if(m_dwHigherRestriction !=0 && who->m_pcAvatar->m_cStats.m_dwLevel > m_dwHigherRestriction )
	{
		cMasterServer::ServerMessage(ColorBlue,NULL,"You are to powerful to use this portal!");
	}
	else if (who->m_pcAvatar->m_cStats.m_dwLevel < m_dwLowerRestriction)
	{
		cMasterServer::ServerMessage(ColorBlue,NULL,"You are to weak to use this portal!");
	}
	else
	{
		cWorldManager::TeleportAvatar( who, m_cDestination );
	}

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of portal objects.
 *
 *	This function is called whenever a portal is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cPortal::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	DWORD flags = 0x00000009;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID;

	//Cubem0j0:  Check for level restrictions on this portal
	if( m_dwLowerRestriction > 0)
	{
		
		cmAssess	<< flags 
					<< 0x01L				//Success = 0x01, Failure = 0x00
					<< WORD(0x0003)			//Total number of DWORDS
					<< WORD(0x0010)			//Unknown
					<< 0x56L				//Min Level
					<< DWORD(this->m_dwLowerRestriction)
					<< 0x57L				//Max Level
					<< DWORD(this->m_dwHigherRestriction)
					<< 0x6FL				//Portal Restriction flags
					<< 0x01L				// ?

					<< WORD(0x0002)			//Count (as above) total number of DWORDS
					<< WORD(0x0008)			//Unknown
					<< 0x10L				//Long Description (for portals we use the name)
					<< this->m_strName.c_str()
					<< 0x26L				//Destination:
					<< this->m_strDescription.c_str();

	}
	else
	{
		cmAssess	<< flags 
					<< 0x01L				//Success = 0x01, Failure = 0x00
					<< WORD(0x0001)			//Total number of DWORDS
					<< WORD(0x0010)			//Unknown


					<< 0x6FL				//Portal Restriction flags
					<< 0x01L				// ?

					<< WORD(0x0002)			//Count (as above) total number of DWORDS
					<< WORD(0x0008)			//Unknown
					<< 0x10L				//Long Description (for portals we use the name)
					<< this->m_strName.c_str()
					<< 0x26L				//Destination:
					<< this->m_strDescription.c_str();
	}

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);
	
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}