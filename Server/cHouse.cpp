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
 *	@file cHouse.cpp
 *	Implements functionality for house objects.
 *
 *	This class is referenced whenever a house object is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "object.h"
#include "masterserver.h"
#include "worldmanager.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of houses.
 *
 *	Called whenever a house object should be initialized.
 *
 *	Generally, house objects reside where the house models are located in the world.
 *	By default, the houses are inaccessable (restricted client-side) by characters unless the proper house object is created.
 */
cHouse::cHouse( char *szName, char *szDescription, WORD type, DWORD dwGUID, cLocation *pcLoc )
{
	SetLocation( pcLoc );
	m_dwGUID = dwGUID;
	m_strName.assign( szName );
	m_strDescription.assign( szDescription );
	m_dwModel = type;
	m_fStatic = TRUE;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of house objects in the world.
 *
 *	This function is called whenever a house object should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cHouse::CreatePacket()
{
	cMessage cmReturn;
	
	float	flpScale;		// Scale used in Packet being Sent
	float	flmScale = 0.1f;// Model Data Scale

	/*
	wModelID
	0x4720	apartment
	0x4727	apartment
	0x5130	cottage
	0x512E	cottage
	0x512F	cottage
	0x36A0	cottage
	0x37A8	cottage
	0x2920	villa
	0x218B	mansion

	wIconID
	0x2181	apartment
	0x2181	cottage
	0x218E	villa
	0x218B	mansion
	*/

	cmReturn	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );

	DWORD dwFlags1 = 0x00028081;// Flags 1 defines whats next
	cmReturn << dwFlags1;

	cmReturn << 0x00000034;		// Unknown
	
	// Flags1 Mask 0x00028081
	{
		// Flags1 & 0x00020000 -- Unknown DWORD
		cmReturn << 0x00000065;

		// Flags1 & 0x00008000 -- Location 
		cmReturn.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );	//Next comes the location

		// Flags1 & 0x00000001
			//DWORD dwModel = 0x02000001; // the model.
			DWORD dwModel = 0x02000000L + m_dwModel;
			cmReturn << dwModel;	// dwModel;

		// Flags1 & 0x00000080 -- unknown_green
		flpScale = 0.1f;		// FLOAT flpScale
		cmReturn << flpScale;
	}
	//Next comes some unknown flags...these flags contain information such
	//as whether or not the object is solid (collision detection), the radar color
	//PK information, and more.  I will crack these some time

	// SeaGreens
	WORD wUnkFlag2 = 0x0000;
	WORD wUnkFlag3 = 0x0000;
	WORD wUnkFlag4 = 0x0000;
	WORD wUnkFlag6 = 0x0000;
	WORD wUnkFlag7 = 0x0000;
	WORD wUnkFlag8 = 0x0000;
	WORD wUnkFlag10 = 0x0000;

	cmReturn	<< WORD(0x0001)		// m_wPositionSequence // movement 0x0001 (can also be 0x0000)
				<< wUnkFlag2		// animations
				<< wUnkFlag3		// bubble modes
				<< wUnkFlag4		// num jumps
				<< m_wNumPortals 
				<< wUnkFlag6		// anim count
				<< wUnkFlag7		// overrides
				<< wUnkFlag8

				<< WORD(0x0D7F)		// m_wNumLogins // 0x0D4B
				<< wUnkFlag10;

	char	szCommand[200];
	RETCODE	retcode;

	// Flags2 -- Defines what data comes next

	DWORD dwFlags2;
	if (m_dwOwnerID) {
		dwFlags2 = 0x0E200010;	// mask for owned dwelling
	} else {
		dwFlags2 = 0x0C200010;	// mask for unowned dwelling (excludes owner)
	}

	cmReturn << dwFlags2;
	
	cmReturn << Name( );	// Object's Name
	
	cmReturn << m_wModel << m_wIcon;

	DWORD dwObjectFlags1 = 0x00000080; // 0x0080 Miscellaneous Object
	DWORD dwObjectFlags2 = 0x00000094; // 0x0004 - Cannot be picked up, 0x0080 Unknown - Cannot be selected 0x0010 Unknown - Can be selected.
	// appears to contradict it self; sets Cannot be Selected and Can be Selected.  Housing Item, may set house open/closed

	cmReturn << dwObjectFlags1 << dwObjectFlags2;
	
	// Flags2 & 0x00000010 -- DWORD unknown10
	cmReturn << 0x00000001;
	
	// Flags2 & 0x08000000 -- WORD unknown8000000
		cmReturn << WORD(0x0098);

	// Flags2 & 0x00200000 -- WORD burden
	cmReturn << WORD(0x000A);	// total burden of this object

	if (dwFlags2 == 0x0E200010)	// if masked for owned dwelling
	{
		// Flags2 & 0x02000000	// ObjectID owner
		cmReturn << m_dwOwnerID;// the owner of the object
	}

	WORD	wGuestCount = 0;
	char	GuestIDBuff[9];
	DWORD	GuestID = NULL;
	WORD	StorageOpen;

	sprintf( szCommand, "SELECT COUNT(ID) FROM houses_guest_lists WHERE HouseID=%d;",m_wModel );
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wGuestCount, sizeof( wGuestCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
											
	if( retcode == SQL_NO_DATA )
		wGuestCount = 0;

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	// Flags2 & 0x04000000		// Dwelling access control list
	cmReturn	<< 0x10000002	// DWORD flags -- believed to be flags that control the size and content of this structure
				<< IsOpen		// DWORD open:  0 = private dwelling, 1 = open to public
				<< 0x00000000	// ObjectID allegiance -- allegiance monarch (if allegiance access granted)
				<< wGuestCount	// WORD guestCount -- number of guests on list
				<< WORD(0x0300);// WORD guestLimit -- Maximum number of guests on guest list (cottage is 32)

	sprintf( szCommand, "SELECT GuestGUID,StorageAccess FROM houses_guest_lists WHERE HouseID=%d;",m_wModel );										
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, GuestIDBuff, sizeof( GuestIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_USHORT, &StorageOpen, sizeof( WORD ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		sscanf(GuestIDBuff,"%08x",&GuestID);
		cmReturn << DWORD(GuestID) << StorageOpen;	//0x0000 = access to dwelling; 0x0001 = access also to storage;
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	return cmReturn;
}

/**
 *	Handles the actions of house objects.
 *
 *	This function is called whenever a house object is used or should perform an action.
 */
void cHouse::Action(cClient* who)
{
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of house objects.
 *
 *	This function is called whenever a house object is assessed by a client.
 *	House objects are intended to be imperceptible to the client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cHouse::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x00000080L << 1L;
	cmAssess << 0xEL;
	cmAssess << 30L << 50L << 50L;
	cmAssess << 60L << 40L << 50L << 60L << 100L << 100L << 70L << 150L << 70L << 150L << 1L;
	cmAssess << 0xFFFFFFFFL << 0xFFFFFFFFL;
	cmAssess << 50L << 100L << 0L;
	cmAssess << "Male" << "Isparian" << "NPC" << "" << "" << "" << 1L << 2L << 1L << 3L << 2L << 3L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
	
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}