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
 *	@file Database.h
 *	Defines functions relating to the database.
 */

#ifndef __DATABASE_H
#define __DATABASE_H

#include <sql.h>
#include <sqlext.h>
#include <sstream>
#include <vector>

#include "Account.h"
#include "Shared.h"

class cAvatar;

class cDatabase
{

public:	

	static HENV		m_hEnv;
	static HDBC		m_hDBC;							
	static HSTMT	m_hStmt;
	static int		wMaxModel;
	static char szModelName[1000][5];
	static char	szModelNumber[1000][3];
	static char szDBIP[16];
	static char szDBNAME[20];
	static char	szDBUSER[20];
	static char	szDBPASSWORD[20];
	static int	intDBType;
	
	/**	@name Database Initialization and Loading
	 *	Functions to initialize or load objects using database information 
	 *	@{
	 */
	static void	Load					( );
	static void Unload					( );
	static void GetError				( SQLSMALLINT type, SQLHANDLE *hptr );
	static int	InitializeSQLEnvironment( );
	static int	FreeSQLEnvironment		( );
	static void cDatabase::InitializeMaxModel( );
	static void cDatabase::LoadModelList( );
	static void InitializeAvatarGUIDs	( DWORD &dwGUIDCycle_Avatar );
	static void InitializeObjectGUIDs	( DWORD &dwGUIDCycle_Object );
	static void InitializeAllegIDs		( DWORD &dwIDAllegiance );
	static BOOL VerifyAccount			( char* szUserName, char* szPassword, DWORD &dwAccountID, BOOL fCreate );
	static BOOL CreateAccount			( char* szUserName, char* szPassword, DWORD &dwAccountID );
	static void LoadAvatarList			( DWORD dwAccountID, std::vector< cAvatarList > &AvatarList );
	static BOOL CreateAvatar			( DWORD dwAccountID, CreateCharacterMessage &ccm, DWORD &dwNewAvatarGUID );
	static void DeleteAvatar			( DWORD dwAvatarGUID );
	static void LoadAvatar				( cAvatar* pcAvatar );
	static BOOL LoadDungeonList			( char strName, std::vector< cDungeonList > &DungeonList );
	static void SetupDB					( int DBType, char* szDBIP, char* szDBNAME, char* szDBUSER, char* szDBPASSWORD );
	//@}
	
	//eLeM:  Functions to update avatar inventory in the database
	static void AddToInventoryDB		( DWORD dwAvatarGUID, cObject* pcObj );
	static void RemoveFromInventoryDB	( DWORD dwAvatarGUID, DWORD dwObjectGUID );

	//k109:  Adding functions to ease adding spells to Avatar
	static void AddSpell				( DWORD dwAvatarGUID, int SpellCount, int SpellID );
	static void AddSpellToTab			(DWORD dwAvatarGUID, int SpellID, int SpellCount, int TabID);
	
	//k109:  Functions to add starting items
	static void AddGeneralItems			(DWORD dwAvatarGUID);
	static void AddRaceItemsAluvian		(DWORD dwAvatarGUID);
	static void AddRaceItemsGharundim	(DWORD dwAvatarGUID);
	static void AddRaceItemsSho			(DWORD dwAvatarGUID);

	static void AddCasterItems			(DWORD dwAvatarGUID);
	static void AddFoci					(DWORD dwAvatarGUID, CreateCharacterMessage &ccm);
	static void AddProfessionItems		(DWORD dwAvatarGUID, CreateCharacterMessage &ccm);

	static void SaveCharacterFlags		(DWORD dwAvatarGUID, DWORD Flags);

	//eLeM:  Functions to convert between numerical types
	static float Hex2Float (DWORD hexValue);
	static DWORD Float2Hex (float floatValue);

	static inline char *ReturnClass( DWORD dwClass )
	{
		switch ( dwClass )
		{
		case CHARDATA_CLASS_CUSTOM:			return "Custom";
		case CHARDATA_CLASS_BOW_HUNTER:		return "Bow Hunter";
		case CHARDATA_CLASS_SWASHBUCKLER:	return "Swashbuckler";
		case CHARDATA_CLASS_LIFE_CASTER:	return "Life Caster";
		case CHARDATA_CLASS_WAR_MAGE:		return "War Mage";
		case CHARDATA_CLASS_WAYFARER:		return "Wayfarer";
		case CHARDATA_CLASS_SOLDIER:		return "Soldier";
		default:							return NULL;
		}
	}

	static inline char *ReturnGender( DWORD dwGender )
	{
		switch ( dwGender )
		{
		case CHARDATA_SEX_FEMALE:	return "Female";
		case CHARDATA_SEX_MALE:		return "Male";
		default:					return NULL;
		}
	}

	static inline char *ReturnRace( DWORD dwRace )
	{
		switch ( dwRace )
		{
		case CHARDATA_RACE_ALUVIAN:	return "Aluvian";
		case CHARDATA_RACE_GHARU:	return "Gharu'ndim";
		case CHARDATA_RACE_SHO:		return "Sho";
		default:					return NULL;
		}
	}

	static inline char *ReturnNamePrefix( BYTE bLevel )
	{
		switch ( bLevel )
		{
		case eDeveloper:	return "+Developer ";
		case eAdmin:		return "+Envoy ";
		case eUeber:		return "+Uber ";
		case eSentinel:		return "+Sentinel ";
		case eAdvocate:		return "+Advocate ";
		case eStaff:		return "+";
		case eVIP:			return "*";
		case eNormal:		return "";
		default:					return NULL;
		}
	}
private:

};

#endif	// #ifndef __DATABSE_H