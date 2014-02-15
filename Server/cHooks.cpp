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
 *	@file cHooks.cpp
 *	Implements functionality for housing hooks.
 *
 *	This class is referenced whenever a housing hook is created, used, or assessed.
 *	Inherits from the cObject class.
 */

//#include "Client.h"
#include "MasterServer.h"
#include "Object.h"
#include "WorldManager.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of housing hooks.
 *
 *	Called whenever a housing hook object should be initialized.
 */
cHooks::cHooks( WORD type, DWORD dwGUID, DWORD dwHouseID, char *szName, char *szDescription, cLocation *pcLoc )
{
	SetLocation( pcLoc );
	m_dwGUID = dwGUID;
	m_strName.assign( szName );
	m_strDescription.assign( szDescription );
	m_dwModel = type;
	m_fStatic = TRUE;
	m_dwHouseID = dwHouseID;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of housing hooks in the world.
 *
 *	This function is called whenever a housing hook should be created for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cHooks::CreatePacket()
{
	cMessage cmCreate;

	char	szCommand[200];
	RETCODE	retcode;
	
	char	OwnerIDBuff[9];
	DWORD	OwnerID = 0x0L;

	sprintf( szCommand, "SELECT OwnerID FROM houses_covenants WHERE HouseID = %d;",m_dwHouseID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	// Return SQL_SUCCESS if there is a house that corresponds to the hook's HouseID
	if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) {
		sscanf(OwnerIDBuff,"%08x",&OwnerID);
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );


	float	flpScale;				// Scale used in Packet being Sent
	float	flmScale = 0;			// Model Data Scale
	DWORD	dwFlags1 = 0x00009081;  // Flags1
	
	/*
	dwModel
	0x0A8D	Floor Hook
	0x0A8E	Wall Hook
	0x0A8C	Ceiling Hook
	0x0A8D	Yard Hook
	0x0A8D	Roof Hook

	wModelID
	0x2DB1	Floor Hook
	0x25D6	Wall Hook
	0x2DB2	Ceiling Hook
	0x3187	Yard Hook
	0x3186	Roof Hook

	wIconID
	0x20C0
	*/

	WORD tBurden = 0;

	if (IsUsed == 1)
	{
//		cItemModels *h_model = cItemModels::FindModel(m_hook_item->GetItemModelID());
//		h_model->m_wBurden		//Burden (WORD, no padding)
//		tBurden += h_model->m_wBurden;
	}
/*
		//Loop through inventory and find all items.				
		for (int i = 0; i < this->m_hook_lstInventory.size(); i++)
		{
			//Find each item by it's GUID
			cObject *v_item = this->FindInventory(this->v_guids[i]);

			//Associate model with item
			cItemModels *v_model = cItemModels::FindModel(v_item->GetItemModelID());
//			v_item->GetGUID()		//Item GUID
//			v_model->m_dwFlags2		//10254018
//			v_model->m_strName.c_str()	//Item Name
//			v_model->m_wModel		//Model
//			v_model->m_wIcon		//Icon
//			v_model->m_wBurden		//Burden (WORD, no padding)
//			v_model->m_wHooks;		//Hook Type (WORD, no padding)
		}
*/
	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );

	cmCreate << dwFlags1;

	cmCreate << WORD(0x0014);	// Type of portal 
	cmCreate << WORD(0x0000);	// Unknown
	
	//Flags1 Mask: 0x00009081
	{
		//Flags1 & 0x00008000 -- Location 
		cmCreate.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );	//Next comes the location

		//Flags1 & 0x00001000 -- DWORD ResourceID
		cmCreate << 0x3400002B;		// unknown ResourceID

		//Flags1 & 0x00000001 -- DWORD dwModel = 0x02000001; // the model.
		DWORD dwModel = 0x02000000L + m_dwModel;
		cmCreate << dwModel;

		//Flags1 & 0x00000080 -- unknown_green
			
		if (flmScale > 0)
		{
			flpScale = flmScale;// FLOAT flpScale
		}
		else
		{
			flpScale = 0.5;
		}
		cmCreate << flpScale;
	}

	// SeaGreens
	WORD wUnkFlag2 = 0x0000;
	WORD wUnkFlag3 = 0x0002;
	WORD wUnkFlag4 = 0x0000;
	WORD wUnkFlag6 = 0x0000;
	WORD wUnkFlag7 = 0x0000;
	WORD wUnkFlag8 = 0x0000;
	WORD wUnkFlag10 = 0x0000;

	cmCreate	<< m_wPositionSequence	// movement 0x0001
				<< wUnkFlag2			// animations
				<< wUnkFlag3			// bubble modes
				<< wUnkFlag4			// num jumps
				<< m_wNumPortals 
				<< wUnkFlag6			// anim count
				<< wUnkFlag7			// overrides
				<< wUnkFlag8
				<< WORD(0x7A51)			// m_wNumLogins // 0x0D4B
				<< wUnkFlag10;
	
	DWORD dwFlags2 = 0x3220003A;		// Flags2 -- Defines what data comes next

	cmCreate << dwFlags2;
	
	cmCreate << Name( );	// Object's Name
	
	cmCreate << WORD(m_wModel) << WORD(m_wIcon);

	DWORD dwObjectFlags1 = 0x00000200; // 0x0200 Containers
	DWORD dwObjectFlags2 = 0x00000015; // 0x0010 is not an npc; 0x0004 cannot be picked up; 0x0001 can be opened (false if locked) 

	cmCreate << dwObjectFlags1 << dwObjectFlags2;

	// Flags2 & 0x00000002 -- BYTE itemSlots
	cmCreate << BYTE(0x01);		// number of item slots

	// Flags2 & 0x00000008 -- BYTE value
	cmCreate << BYTE(0x0A);		// object value

	// Flags2 & 0x00000010 -- DWORD unknown10
	cmCreate << 0x30000000L;
	
	// Unknown
	cmCreate	<< BYTE( 0 )
				<< BYTE( 0 )
				<< BYTE( 0 );

	// Flags2 & 0x00000020 -- float approachDistance
	float flApproach = 10.0f;	// 0x41200000
	cmCreate << flApproach;		// distance a player will walk to use the object

	// Flags2 & 0x00200000 -- WORD burden
	tBurden += WORD(0x0005);
	cmCreate << tBurden;		// total burden of this object

	// Flags2 & 0x02000000 -- ObjectID owner
	cmCreate << OwnerID;		// the owner of this object

	// Flags2 & 0x20000000 -- hookType Unknown
	cmCreate << WORD(0xFFFF);	// always -1

	// Flags2 & 0x10000000 -- hookable On
/*
	0x0001	floor hook
	0x0002	wall hook
	0x0004	ceiling hook
	0x0008	yard hook
	0x0010	roof hook
*/
	cmCreate << WORD(0xFFFF);	// the types of hooks this object may be placed on (-1 for hooks)

	return cmCreate;
}

/**
 *	Handles the actions of housing hook objects.
 *
 *	This function is called whenever a housing hook is used or should perform an action.
 */
void cHooks::Action(cClient* who)
{
	cMessage cmUseHook;
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

	cmUseHook	<< 0xF74CL					// Packet type
				<< who->m_pcAvatar->GetGUID()	// Character's GUID
				<< m_wNumLogins				// number of logins
				<< ++m_wPositionSequence	// sequence number
				<< ++m_wNumAnims			// Number Animations this login session
				<< WORD(0x00)				// Activity
				<< WORD(0x08)
				<< WORD(0x3D)				// Starting Stance
				<< m_dwGUID;				// Hook's GUID
	cmUseHook	<< 0x4342BD54				// DWORD unknown_value
				<< 0x19C5EE0F				// bitfield?
				<< 0x3F800000				// unknown -- unknown DWORD
				<< 0x00000000;				// float heading -- Appears to be the heading to which the object is turning
	
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmUseHook, 3 );

	if(m_wState == 0)
	{
		//Display the item slot in the hook (0x0196)
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
 *	Handles the assessment of housing hook objects.
 *
 *	This function is called whenever a housing hook is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cHooks::Assess(cClient *pcAssesser)
{
//	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );
	cMessage cmAssess;
	DWORD flags = 0x0000000B;
	m_strDescription.assign("Use this item to open it and see its content");

	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags
	<< 0x01L;					// Success = 0x01; Failure = 0x00

	// Flags & 0x00000001
	WORD dwUnkCount = 0x0002;
	cmAssess << dwUnkCount		// Total number of DWORDS
	<< WORD(0x0010)				// Unknown
		<< 0x13L				// Value
		<< 0x0AL				// DWORD value
		<< 0x05L				// Burden
		<< 0x05L				// DWORD value

	// Flags & 0x00000002
	<< WORD(0x0001)				// Total number of DWORDS
	<< WORD(0x0008)				// Unknown	
		<< 0x02L				// Value
		<< 0x00L				// DWORD value

	// Flags & 0x00000008
	<< WORD(0x0002)				// Total number of DWORDS
	<< WORD(0x0008)				// Unknown	
		<< 0x10L;				// DWORD Full Description
		cmAssess << "This hook is owned by Cubem0j0"
		<< 0x0EL				// DWORD Usage Instructions
		<< this->m_strDescription.c_str();

	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}