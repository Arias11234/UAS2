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
 *	@file cCovenantCrystal.cpp
 *	Implements functionality for covenant crystals.
 *
 *	This class is referenced whenever a covenant crystal is created, used, or assessed.
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
 *	Handles the creation of covenant crystals.
 *
 *	Called whenever a covenant crystal object should be initialized.
 */
cCovenant::cCovenant( WORD type, DWORD dwGUID, DWORD dwHouseID, char *szName, char *szDescription, cLocation *pcLoc )
{
	SetLocation( pcLoc );
	m_dwGUID = dwGUID;
	m_dwHouseID = dwHouseID;
	m_strName.assign( szName );
	m_strDescription.assign( szDescription );
	m_dwModel = type;
	m_fStatic = TRUE;
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of covenant crystals in the world.
 *
 *	This function is called whenever a covenant crystal should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cCovenant::CreatePacket()
{
	cMessage cmCreate;
	
	char	szCommand[200];
	RETCODE	retcode;
	
	char	OwnerIDBuff[9];
	DWORD	OwnerID = NULL;

	char	szName[75];
	std::string		strName = m_strName;

	sprintf( szCommand, "SELECT OwnerID FROM houses_covenants WHERE HouseID = %d;",m_dwHouseID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) {		
		sscanf(OwnerIDBuff,"%08x",&OwnerID);

		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

		sprintf( szCommand, "SELECT Name FROM avatar WHERE AvatarGUID = %08x;",OwnerID);
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
	
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, szName, sizeof( szName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

		// Return SQL_SUCCESS if there is a player that corresponds to owner of the fetched covenant crystal
		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) {
		
			std::string		str1 = szName;
  			strName.assign(str1 + "'s " + m_strName);
		}
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	float	flpScale;				// Scale used in Packet being Sent
	float	flmScale = 0;			// Model Data Scale
	DWORD	dwFlags1 = 0x00018003;  // Flags1

	if (WORD(m_wModel) != 0x3CF8) {	// if not apartment (cottages, villas, mansions)
		flmScale = 1.2f;
		dwFlags1 += 0x00000080L;	// mask 'floatscale' if cottage, villa, or mansion
	}
/*
AnimConfig
0x00B8L	cottages,villas,mansions
0x00EAL	apartment

dwModel
0x0AAFL	cottages,villas,mansions
0x0C7AL	apartment

wModelID		
0x2DC2	mansion
0x2ECC	villa
0x2DC5	villa	
0x5173	cottage
0x3CF8	apartment

wIconID
0x218C
*/
	cmCreate	<< 0xF745L 
				<< m_dwGUID 
				<< BYTE( 0x11 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 ) 
				<< BYTE( 0 );

	cmCreate << dwFlags1;

	cmCreate << 0x00000414;	// Type of portal (0x0410 = solid; 0x0414 = not solid?)
	

	// Flags1 Mask: 0x00018003 or 0x00018083
	{
		// Flags1 & 0x00100000 -- Unknown Count
		DWORD dwUnkCount = 0x0000000C;

		BYTE ownedByte;
		if (OwnerID) {
			ownedByte = 0x0B;
		} else {
			ownedByte = 0x0A;
		}
		BYTE unknownByte[0x0C] = {	0x00,0x00,0x3D,0x00,0x02,0x00,0x00,0x00,
									ownedByte, // model appearance:  owned = 0x0B; unowned = 0x0A, 0x0C?
									0x00,0x00,0x00};
		cmCreate << dwUnkCount;
			
		for ( int i = 0; i < dwUnkCount; ++i ) {
			cmCreate << unknownByte[i];
		}
		DWORD dwUnknownDword = 0x0L;
		cmCreate << dwUnknownDword;
		
		// Flags1 & 0x00008000 -- Location 
		cmCreate.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );	// the location

		//Flags1 & 0x0000002 -- DWORD ResourceID Animations
		DWORD dwAnimC = 0x09000000L + m_wAnimConfig;
		cmCreate << dwAnimC;

		// Flags1 & 0x00000001 -- DWORD dwModel = 0x02000001; // the model.
		DWORD dwModel = 0x02000000L + m_dwModel;
		cmCreate << dwModel;

		if (WORD(m_wModel) != 0x3CF8)
		{
			//Flags1 & 0x00000080 -- unknown_green
			if (flmScale > 0)
			{
				flpScale = flmScale;	// FLOAT flpScale
			}
			else
			{
				flpScale = 1.0;
			}
			cmCreate << flpScale;
		}
	}

	// SeaGreens
	WORD wUnkFlag2 = 0x0004;
	WORD wUnkFlag3 = 0x0000;
	WORD wUnkFlag4 = 0x0000;
	WORD wUnkFlag6 = 0x0004; // always seems same as wUnkFlag2
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
				<< WORD(0x0D7F)			// m_wNumLogins // 0x0D4B
				<< wUnkFlag10;

	
	DWORD dwFlags2 = 0x0030L;		// Flags2 -- Defines what data comes next
	if (m_dwOwnerID) {
		dwFlags2 += 0x02000000L;	// mask 'houseOwnerID' if owned
	}

	cmCreate << dwFlags2;
	
	cmCreate << strName.c_str();	// Object's Name
	
	cmCreate << WORD(m_wModel) << WORD(m_wIcon);

	DWORD dwObjectFlags1 = 0x00000000; // 0x0080 -- Miscellaneous Object
	DWORD dwObjectFlags2 = 0x00001014; // 0x0004 -- Cannot be picked up; 0x0010 -- Can be selected

	cmCreate << dwObjectFlags1 << dwObjectFlags2;

	// Flags2 & 0x00000010
	DWORD dwunknown_v2 = 0x00000020; // Value
	cmCreate << dwunknown_v2;

	// Flags2 & 0x00000020
	float flApproach = 3.0f; 
	cmCreate << flApproach;

	if (OwnerID) {
		// Flags2 & 0x02000000
		cmCreate << OwnerID;
	}
	return cmCreate;
}

/**
 *	Handles the actions of covenant crystal objects.
 *
 *	This function is called whenever a covenant crystal is used or should perform an action.
 */
void cCovenant::Action(cClient* who)
{
	cMessage cmUseCrystal;

	char	szCommand[200];
	RETCODE	retcode;
	
	char	OwnerIDBuff[9];
	DWORD	OwnerID = NULL;
	char	HouseGUIDBuff[9];
	DWORD	HouseGUID = NULL;

	char	szName[75];
	std::string		strPlayerName = "";

	sprintf( szCommand, "SELECT OwnerID,GUID FROM houses WHERE wModel = %d;",m_dwHouseID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_CHAR, HouseGUIDBuff, sizeof( HouseGUIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	// Return SQL_SUCCESS if there is a house that corresponds to the covenant crystal
	if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) {
		
		sscanf(OwnerIDBuff,"%08x",&OwnerID);
		sscanf(HouseGUIDBuff,"%08x",&HouseGUID);

		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );


		sprintf( szCommand, "SELECT Name FROM avatar WHERE AvatarGUID = %08x;",OwnerID);
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
	
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, szName, sizeof( szName ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

		// Return SQL_SUCCESS if there is a player that corresponds to owner of the fetched house
		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS) {
			std::string		str1 = szName;
  			strPlayerName.assign(str1);
		}
		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
	} else {
		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
	}

	cmUseCrystal<< 0xF74CL					// Packet type
				<< who->m_pcAvatar->GetGUID()	// Character's GUID
				<< m_wNumLogins				// number of logins
				<< ++m_wPositionSequence	// sequence number
				<< ++m_wNumAnims			// Number Animations this login session
				<< WORD(0x00)				// Activity
				<< WORD(0x06)
				<< WORD(0x3D)				// Starting Stance
				<< m_dwGUID;					// Covenant Crystals's GUID
				cmUseCrystal.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) ); // Covenant Crystals' location
	cmUseCrystal<< 0x19C5EE4F				// bitfield?
				<< 0x40400000				// float1 -- unknown float
				<< 0x00000000				// float2 -- unknown float
				<< 0x7F7FFFFF				// unknown3 -- unknown DWORD
				<< 0x3F800000				// speed (1)
				<< 0x41700000				// float4 -- unknown float
				<< 0x00000000				// heading
				<< 0x3F800000;				// unknown -- unknown DWORD
	
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmUseCrystal, 3 );

	// eLeM:  Display Dwelling Purchase/Maintenance Panel

	int i;
	cMessage cmDisplayPanel;
	cmDisplayPanel << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x021DL 
	<< m_dwGUID;			// Covenant Crystal's GUID

	DWORD ownerType =		0x00000001;	// Self = 0x00000001; Allegiance = 0x00000003
	DWORD levelReq =		0x00000001;	// Apartment, Cottage = 0x00000014; Villa = 0x00000023; Mansion = 0x00000032
	DWORD unknown2 =		0xFFFFFFFF;
	DWORD rankReq =			0xFFFFFFFF;	// Apartment, Cottage, Villa = 0xFFFFFFFF; Mansion = 0x00000006
	DWORD unknown3 =		0xFFFFFFFF;
	DWORD unknown4 =		0x00000000;
	DWORD unknown5 =		0x00000001;
	DWORD ownerName=		0x00000000;
	DWORD purchaseCount =	0x00000001;	// Apartment = 2; Cottage, Villa = 3; Mansion = 6;
		
	cmDisplayPanel	<< HouseGUID	// Dwelling ID
					<< OwnerID		// Dwelling Owner ID (NULL == 0x00000000)
					<< ownerType
					<< levelReq			// level requirement to purchase this dwelling (-1 if no requirement)
					<< unknown2
					<< rankReq			// rank requirement to purchase this dwelling (-1 if no requirement)
					<< unknown3
					<< unknown4
//					<< unknown5
					<< strPlayerName.c_str();		// name of the current owner (NULL == 0x00000000)

	/* Item type:
	0x00000111 = Pyreal
	0x00002DBE = Writ of Refuge / Writs of Refuge
	0x00001086 = Mattekar Hide Sleeves
	0x00000E74 = Gold Phyntos Wasp Wing
	0x00002527 = Golden Gromnie
	0x000021FD = A Lucky Gold Letter
	0x000024C5 = Dread Mattekar Paw
	*/
	DWORD dwPurchaseCount;	// Apartment = 2; Cottage, Villa = 3; Mansion = 6;
	sprintf( szCommand, "SELECT COUNT(ID) FROM houses_purchase WHERE HouseID = %d;",m_dwHouseID );
	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwPurchaseCount, sizeof( dwPurchaseCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	if( retcode == SQL_NO_DATA )
		dwPurchaseCount = 0;
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
	cmDisplayPanel << dwPurchaseCount;		// the number of items required to purchase this dwelling

	/* Item type:
	0x00000111 = Pyreal
	0x00002DBE = Writ of Refuge / Writs of Refuge
	0x00001086 = Mattekar Hide Sleeves
	0x00000E74 = Gold Phyntos Wasp Wing
	0x00002527 = Golden Gromnie
	0x000021FD = A Lucky Gold Letter
	0x000024C5 = Dread Mattekar Paw
	*/

	/* Purchase Amount:
	Apartment:
	0x000186A0	100,000 Pyreal
	0x00000001	1 Writ of Refuge

	Cottage:
	0x000493E0	300,000 Pyreal
	0x00000001	1 Writ of Refuge
		Other Item:
		0x00000001	1 Mattekar Hide Sleeves

	Villa:
	0x001E8480	2,000,000 Pyreal
	0x00000005	5 Writs of Refuge
		Other Item:
		0x00000001	1 Gold Phyntos Wasp Wing

	Mansion:
	0x00989680	10,000,000 Pyreal
	0x00000014	20 Writs of Refuge
		Other Items:
		0x00000014	20 Golden Gromnie
		0x0000000F	15 A Lucky Gold Letter
		0x00000002	2 Dread Mattekar Paw
	*/
	DWORD	dwBuyType;
	DWORD	dwBuyRequired;
	DWORD	dwBuyPaid;
//	String	buyName;
//	String	buyPluralName;

	sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_purchase WHERE HouseID = %d;",m_dwHouseID );														
	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwBuyType, sizeof( dwBuyType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwBuyRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwBuyPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		if( retcode == SQL_NO_DATA ) {
			dwBuyRequired = 1;
			dwBuyPaid = 0;
			dwBuyType = 12;
		}
		cItemModels *pcModel = cItemModels::FindModel(dwBuyType);
		std::string		strPluralName = pcModel->m_strName.c_str();
  		strPluralName.assign(strPluralName + "s");

		cmDisplayPanel	<< dwBuyRequired				// quantity required
						<< dwBuyPaid					// quantity paid
						<< DWORD(pcModel->m_wModel)		// item's object type
						<< pcModel->m_strName.c_str()	// name of this item
						<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	DWORD dwMaintenanceCount;		// Apartment, Cottage = 1; Villa, Mansion = 2;
	sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",m_dwHouseID );
	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFetch( cDatabase::m_hStmt );
	if( retcode == SQL_NO_DATA )
		dwMaintenanceCount = 0;
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
	cmDisplayPanel << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling

	/* Maintenance Amount:
	Apartment:
	0x00002710	10,000 Pyreal

	Cottage:
	0x00007530	30,000 Pyreal

	Villa:
	0x000186A0	100,000 Pyreal
	0x00000002	2 Writs of Refuge

	Mansion:
	0x000F4240	1,000,000 Pyreal
	0x00000010	10 Writs of Refuge
	*/				
	DWORD	dwMaintainType;
	DWORD	dwMaintainRequired;
	DWORD	dwMaintainPaid;
//	String	maintainName;
//	String	maintainPluralName;	
	
	sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",m_dwHouseID );														
	retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintainType, sizeof( dwMaintainType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwMaintainRequired, sizeof( dwMaintainRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwMaintainPaid, sizeof( dwMaintainPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
		if( retcode == SQL_NO_DATA ) {
			dwMaintainRequired = 1;
			dwMaintainPaid = 0;
			dwMaintainType = 12;
		}
		cItemModels *pcModel = cItemModels::FindModel(dwMaintainType);
		std::string		strPluralName = pcModel->m_strName.c_str();
  		strPluralName.assign(strPluralName + "s");

		cmDisplayPanel	<< dwMaintainRequired			// quantity required
						<< dwMaintainPaid				// quantity paid
						<< DWORD(pcModel->m_wModel)		// item's object type
						<< pcModel->m_strName.c_str()	// name of this item
						<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

	who->AddPacket(WORLD_SERVER,cmDisplayPanel,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of covenant crystal objects.
 *
 *	This function is called whenever a covenant crystal is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cCovenant::Assess(cClient *pcAssesser)
{
	// eLeM:  Covenant Crystal assess

	//	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );
	cMessage cmAssess;
	DWORD flags = 0x00000008;
	m_strDescription.assign(m_dwMode ? "The current maintenance has not been paid." : "The current maintenance has been paid.");

	WORD length = (m_dwMode ? 0x002B : 0x0027);
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags
	<< 0x01L						// Success = 0x01, Failure = 0x00
	<< WORD(0x0001)					// Total number of DWORDS
	<< WORD(0x0008)					// Unknown
	<< 0x10L
	<< this->m_strDescription.c_str();

	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}