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

/* Special thanks to Zerot for the "old" experience calculator (http://rain.prohosting.com/obp/calc.html) */
/* Special thanks to Xerxes of Thistledown for the "new" experience passing formulas */

/**
 *	@file Allegiance.cpp
 *	Implements general functionality for allegiances (hierarchical character alliances/monarchies/guilds).
 *
 *	TODO:	Separate static and non-static (instance-based) functions.
 *			(Static functions should probably be in an allegiance manager file.)
 */

#include "Allegiance.h"
#include "Client.h"
#include "Database.h"
#include "MasterServer.h"
#include "WorldManager.h"

cAllegiance::cAllegiance()
{
    m_ID = 0;
    m_Name = "";
    m_LeaderGUID = 0;
    m_MOTD = "";
	m_Size = 0;
	m_CreationTime = 0;
}

cAllegiance::~cAllegiance()
{

}

void cAllegiance::LoadAllegiance(DWORD ID, DWORD leaderGUID, char name[40], char MOTD[255])
{
    m_ID = ID;
	m_LeaderGUID = leaderGUID;
    m_Name = name;
    m_MOTD = MOTD;
	m_Size = 0;
	m_CreationTime = 0;
	
	cMasterServer::m_AllegianceList.push_back( this );
}

void cAllegiance::CreateAllegiance(DWORD leaderGUID)
{
//    m_ID = cWorldManager::NewID_Allegiance();
	m_ID = leaderGUID;
    m_Name = "";
    m_LeaderGUID = leaderGUID;
    m_MOTD = "";
	m_Size = 0;
	m_CreationTime = time (NULL);
	
	char		szCommand[1024];
	RETCODE		retcode;

	sprintf( szCommand, "INSERT INTO `allegiance` ( `AllegianceID`, `Name`, `LeaderGUID`, `MOTD`, `CreationTime` ) VALUES (%lu, '%s', %lu, '%s', %lu);",m_ID, "", m_LeaderGUID, "", time (NULL));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	cMasterServer::m_AllegianceList.push_back( this );
}

cAllegiance* cAllegiance::GetAllegianceByID(DWORD AllegianceID)
{
	for ( std::vector<cAllegiance *>::iterator itr = cMasterServer::m_AllegianceList.begin() ; itr != cMasterServer::m_AllegianceList.end() ; ++itr )
	{
		if ((*itr)->GetID() == AllegianceID)
        {
            return *itr;
        }
	}
	
	return NULL;
}

/**
 *	Loads the allegiance's members into memory.
 *
 *	This function is called when the server is started.
 *	Loading this information at server boot reduces the impact of experience passing.
 *
 *	Allegiance members are loaded into a map with their GUID as key.
 *	Each member is a struct containing pertinent allegiance information.
 */
void cAllegiance::LoadMembersFromDB(cAllegiance* aAllegiance)
{
	char	szCommand[200];
	RETCODE	retcode;

	DWORD	dwGUID;
	char	szName[75];
	BYTE	bGender;
	BYTE	bRace;
	BYTE	bRank;
	WORD	wLoyalty;
	WORD	wLeadership;
	UINT64	dwPassupXP;
	UINT64	dwPendingXP;
	DWORD	dwPatron;
	DWORD	dwFollowers;
	UINT64	dtSwornRL;
	UINT64	dtSwornIG;
//	DWORD	dwVassals;

	DWORD	dwLevel;
	INT64	dtBirth;

	sprintf( szCommand, "SELECT * FROM `allegiance_members` WHERE `AllegianceID` = '%lu'", aAllegiance->m_ID);

	int iCol = 3;
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS);							CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );																CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwGUID, sizeof( DWORD ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, szName, sizeof( szName ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bGender, sizeof( BYTE ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bRace, sizeof( BYTE ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bRank, sizeof( BYTE ), NULL );			CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wLoyalty, sizeof( BYTE ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wLeadership, sizeof( BYTE ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwPassupXP, sizeof( UINT64 ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwPendingXP, sizeof( UINT64 ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwPatron, sizeof( UINT64 ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwFollowers, sizeof( UINT64 ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dtSwornRL, sizeof( UINT64 ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dtSwornIG, sizeof( UINT64 ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
//	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwVassals, sizeof( DWORD ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

	// load members into the allegiance's "member map"
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		Member	newMember;

		for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)	//ensure that the vassal record is cleared
			newMember.m_dwVassals[iVas] = 0;

		newMember.m_dwGUID = dwGUID;
		newMember.m_szName.assign(szName);
		newMember.m_online = false;
		newMember.m_bGender = bGender;
		newMember.m_bRace = bRace;
		newMember.m_bRank = bRank;
		newMember.m_wLoyalty = wLoyalty;
		newMember.m_wLeadership = wLeadership;
		newMember.m_dwPatron = dwPatron;
		newMember.m_dwFollowers = dwFollowers;
		newMember.m_passupXP = dwPassupXP;
		newMember.m_pendingXP = dwPendingXP;
		newMember.m_timeRLSwear = dtSwornRL;
		newMember.m_timeIGSwear = dtSwornIG;

		//aAllegiance->members.push_back(newMember);
		aAllegiance->members.insert(MemberList::value_type(newMember.m_dwGUID, newMember));
		aAllegiance->m_Size++;
	}

	// load members' levels and time of births into the allegiance's "member map"
	std::map<DWORD, Member>::iterator itMembers;
	for ( itMembers = aAllegiance->members.begin() ; itMembers != aAllegiance->members.end() ; ++itMembers )
	{
		sprintf( szCommand, "SELECT `Level`,`Birth` FROM `avatar` WHERE `AvatarGUID` = '%lu'", itMembers->second.m_dwGUID);

		int iCol = 1;
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS);						CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );	
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dwLevel, sizeof( DWORD ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &dtBirth, sizeof( UINT64 ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLFetch( cDatabase::m_hStmt );
		
		itMembers->second.m_dwLevel = dwLevel;
		itMembers->second.m_timeBirth = dtBirth;
		itMembers->second.m_timeIGTotal = 0;		//TODO

		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	
		// determine patron/vassal relationships
		if (itMembers->second.m_dwGUID != aAllegiance->m_LeaderGUID)	// if the member is not the monarch
		{
			// add the member to his/her patron's vassal list.
			Member* memPatron = &aAllegiance->members.find(itMembers->second.m_dwPatron)->second;
			for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
			{
				if (memPatron->m_dwVassals[iVas] == 0)
				{
					memPatron->m_dwVassals[iVas] = itMembers->first;
					break;
				}
			}
		}
	}
}

/**
 *	Inserts a member into the allegiance.
 *
 *	The function is used when a player should be inserted into an allegiance.
 *
 *	@param member - The member record object for the member that should be inserted.
 *	@param dwPatronGUID - The GUID of the new member's patron.
 */
void cAllegiance::InsertMember(Member member, DWORD dwPatronGUID)
{
	members.insert(MemberList::value_type(member.m_dwGUID, member));// insert the member into the allegiance
	m_Size += member.m_dwFollowers + 1;								// update the allegiance size

	AddDirectVassal(dwPatronGUID, member.m_dwGUID);					// add the vassal to the patron's vassal list
}

/**
 *	Erases a member from the allegiance.
 *
 *	The function is used when a player should be deleted from an allegiance.
 *
 *	@param member - A pointer to the member record that should be deleted.
 */
void cAllegiance::EraseMember(Member* member)
{
	RemDirectVassal(member->m_dwGUID);	// remove the vassal from the patron's vassal list

	m_Size -= member->m_dwFollowers + 1;// update the allegiance size
	members.erase(member->m_dwGUID);	// erase the member from the allegiance
}

/**
 *	Erases a member from the allegiance.
 *
 *	The function is used when a player should be deleted from an allegiance.
 *
 *	@param dwMemberGUID - The GUID of the player whose member record should be deleted.
 */
void cAllegiance::EraseMember(DWORD dwMemberGUID)
{
	Member* member = FindMember(dwMemberGUID);

	RemDirectVassal(dwMemberGUID);		// remove the vassal from the patron's vassal list
		
	m_Size -= member->m_dwFollowers + 1;// update the allegiance size
	members.erase(dwMemberGUID);		// erase the member from the allegiance
}

/**
 *	Adds a player (and any vassals under him/her) to an allegiance.
 *
 *	The function is used when a player pledges allegiance to a member of an allegiance.
 *	Each follower of that member should also be added to the allegiance.
 *	The "old" allegiance to which these members belonged should be deleted if the pledged member was the monarch.
 *
 *	@param dwAvatarGUID - The GUID of the player to add.
 *	@param dwPatronGUID - The GUID of the new patron of the player to add.
 *	@param oldAllegianceID - The ID of player's former allegiance, if any.
 *	@param newAllegianceID - The ID of player's new allegiance.
 */
bool cAllegiance::AddNewMember(DWORD dwAvatarGUID, DWORD dwPatronGUID, DWORD oldAllegianceID, DWORD newAllegianceID)
{
	cAllegiance* oldAllegiance = GetAllegianceByID(oldAllegianceID);
	cAllegiance* newAllegiance = GetAllegianceByID(newAllegianceID);

	if (oldAllegiance && newAllegiance)
	{	// already a member of an allegiance

		// if the player is already a member of an allegiance, he/she was the monarch
		// merge the player and his/her followers into the new allegiance
		Member* oldMember = oldAllegiance->FindMember(dwAvatarGUID);
		SwitchMonarchAndVassals(oldMember, oldAllegiance->GetID(), newAllegiance->GetID());

		Member* newMember = newAllegiance->FindMember(dwAvatarGUID);

		// add the vassal to the patron's vassal list (also adjusts the patron's follower count and rank)
		newAllegiance->AddDirectVassal(dwPatronGUID, dwAvatarGUID);
		newMember->m_dwPatron = dwPatronGUID;

		UpdateMemberPatronDB(dwAvatarGUID, dwPatronGUID);

		return true;

	}
	else
	{	// not already a member of an allegiance

		cAvatar* pcAvatar;

		if (newAllegiance)	// the patron has an allegiance
			pcAvatar = cClient::FindAvatar(dwAvatarGUID);
		else				// the patron does not have an allegiance
			pcAvatar = cClient::FindAvatar(dwPatronGUID);
			//pcAvatar = cClient::FindAvatar(dwAvatarGUID);

		if (pcAvatar)
		{
			Member	newMember;

			for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)	// ensure that the vassal record is cleared
				newMember.m_dwVassals[iVas] = 0;

			newMember.m_dwGUID = pcAvatar->GetGUID();
			newMember.m_szName.assign(pcAvatar->Name());
			newMember.m_online = true;
			if (pcAvatar->GetGender() == 0)
				newMember.m_bGender = 2;
			else
				newMember.m_bGender = 1;
			newMember.m_bRace = pcAvatar->GetRace() + 1;
			newMember.m_bRank = 1;
			newMember.m_wLoyalty = pcAvatar->GetTotalLoyalty();
			newMember.m_wLeadership = pcAvatar->GetTotalLeadership();
			newMember.m_dwPatron = dwPatronGUID;
			newMember.m_dwFollowers = 0;
			newMember.m_passupXP = 0;
			newMember.m_pendingXP = 0;
			newMember.m_timeRLSwear = time(NULL);
			newMember.m_timeIGSwear = 0;

			if (!newAllegiance)
			{	// the patron does not have an allegiance; create one

				// create an allegiance
				cAllegiance* newAllegiance = new cAllegiance();
				newAllegiance->CreateAllegiance(dwPatronGUID);
				
				// add the patron
				newAllegiance->InsertMember(newMember, 0);

				// add the new member record
				AddMemberRecordDB(newAllegiance->GetID(), dwPatronGUID, 0);

				// add the vassal by recursing to this function
				AddNewMember(dwAvatarGUID, dwPatronGUID, oldAllegianceID, newAllegiance->GetID());

				// update the patron's allegiance ID in memory
				pcAvatar->m_dwAllegianceID = newAllegiance->GetID();

				// update the patron's allegiance ID in the database
				UpdateAvatarAllegIDDB(pcAvatar->GetGUID(), newAllegiance->GetID());
			}
			else
			{	// the patron has an allegiance; add the sworn member to it

				newAllegiance->InsertMember(newMember, dwPatronGUID);

				// add the new member record
				AddMemberRecordDB(newAllegiance->GetID(), pcAvatar->GetGUID(), dwPatronGUID);

				// update the avatar's allegiance ID in memory
				pcAvatar->m_dwAllegianceID = newAllegiance->GetID();

				// update the avatar's allegiance ID in the database
				UpdateAvatarAllegIDDB(pcAvatar->GetGUID(), newAllegiance->GetID());
			}

			return true;
		}
	}

	return false;
}

/**
 *	Adds a removed player (and all of his/her followers) to a new allegiance.
 *
 *	The player and the vassals under the player switching are switched.
 *
 *	@param member - The member whose allegiance should be switched.
 *	@param oldAllegianceID - The ID of the member's former allegiance.
 *	@param newAllegianceID - The ID of the member's new allegiance.
 */
bool cAllegiance::AddRemMember(Member* member, DWORD oldAllegianceID, DWORD newAllegianceID)
{
	cAllegiance* oldAllegiance = GetAllegianceByID(oldAllegianceID);
	cAllegiance* newAllegiance = GetAllegianceByID(newAllegianceID);

//	Member* member = this->Find(memberGUID);

	if (oldAllegiance && newAllegiance)
	{
		member->m_dwPatron = 0;
		newAllegiance->InsertMember((*member), 0);
				
		UpdatePlayerAllegID(member->m_dwGUID, newAllegianceID);

		SwitchNonMonarchVassals(member, oldAllegianceID, newAllegianceID);			
		
		UpdateMemberPatronDB(member->m_dwGUID, 0);

		return true;
	}

	return false;
}

/**
 *	Removes a player (and any vassals under him/her) from an allegiance.
 *
 *	The function is used when a player breaks or is kicked/booted/banned from the allegiance.
 *	Each follower of that member should also be removed from the allegiance.
 *
 *	@param dwRemMemberGUID - The GUID of the player to remove.
 *	@param isDisbanding - A boolean value representing whether to disband the allegiance.
 */
bool cAllegiance::RemMember(DWORD dwRemMemberGUID, DWORD oldAllegianceID, bool isDisbanding)
{
	cAllegiance* oldAllegiance = GetAllegianceByID(oldAllegianceID);

	Member* member = oldAllegiance->FindMember(dwRemMemberGUID);

	if (member)
	{
		Member* memPatron = oldAllegiance->FindMember(member->m_dwPatron);

		if (oldAllegiance->GetVassalCount(member) > 0)
		{	// has followers; therefore, monarch of a new allegiance

			oldAllegiance->RemDirectVassal(member->m_dwGUID);	// remove the vassal from the patron's vassal list

			cAllegiance* newAllegiance = new cAllegiance();
			newAllegiance->CreateAllegiance(dwRemMemberGUID);
			newAllegiance->AddRemMember(member, oldAllegianceID, newAllegiance->GetID());

			oldAllegiance->m_Size -= member->m_dwFollowers + 1;	// update the allegiance size
			oldAllegiance->members.erase(member->m_dwGUID);		// erase the member from the allegiance
		}
		else
		{	// no longer in an allegiance
			UpdatePlayerAllegID(dwRemMemberGUID, 0);

			oldAllegiance->EraseMember(member);
		}

		if (isDisbanding)
		{
			oldAllegiance->Disband();
		}
		else
		{
			if (memPatron)
			{
				if (memPatron->m_dwGUID == oldAllegiance->GetLeader())
				{	// the patron was the monarch
					if (oldAllegiance->GetVassalCount(memPatron) == 0)
					{	// the patron's/monarch's only vassal was removed
						RemMember(memPatron->m_dwGUID, oldAllegianceID, true);
					}
				}
			}
		}

		return true;
	}

	return false;
}

/**
 *	Switches a player's (and all of his/her followers') allegiance.
 *
 *	It is prefered over the non-monarch implementation, as it iterates rather than searches through the member map.
 *
 *	@param member - The monach whose allegiance should be switched.
 *	@param oldAllegianceID - The ID of the monarch's former allegiance.
 *	@param newAllegianceID - The ID of the allegiance to which the monarch and his/her followers should be switched.
 */
bool cAllegiance::SwitchMonarchAndVassals(Member* member, DWORD oldAllegianceID, DWORD newAllegianceID)
{
	cAllegiance* oldAllegiance = GetAllegianceByID(oldAllegianceID);
	cAllegiance* newAllegiance = GetAllegianceByID(newAllegianceID);

//	Member* member = this->Find(memberGUID);

	if (oldAllegiance && newAllegiance)
	{
		int switchedMembers = 0;

		//this->members.insert(curAllegiance->members.begin(),curAllegiance->members.end());
		std::map<DWORD, Member>::iterator itrMember;
		for ( itrMember = oldAllegiance->members.begin() ; itrMember != oldAllegiance->members.end() ; )
		{
			newAllegiance->members.insert(MemberList::value_type(itrMember->first, itrMember->second));

			UpdatePlayerAllegID(itrMember->first, newAllegianceID);

			switchedMembers++;

			oldAllegiance->members.erase(itrMember++);
		}

		newAllegiance->m_Size += switchedMembers;				//update the new allegiance's member count

		oldAllegiance->Disband();	// delete the former allegiance object

		return true;
	}

	return false;
}

/**
 *	Switches a player's followers' allegiance.
 *
 *	Only the vassals under the player switching are switched.
 *
 *	@param member - The member whose allegiance should be switched.
 *	@param oldAllegianceID - The ID of the member's former allegiance.
 *	@param newAllegianceID - The ID of the member's new allegiance.
 */
bool cAllegiance::SwitchNonMonarchVassals(Member* member, DWORD oldAllegianceID, DWORD newAllegianceID)
{
	cAllegiance* oldAllegiance = GetAllegianceByID(oldAllegianceID);
	cAllegiance* newAllegiance = GetAllegianceByID(newAllegianceID);

//	Member* member = this->Find(memberGUID);

	if (oldAllegiance && newAllegiance)
	{
		for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
		{
			if (member->m_dwVassals[iVas] != 0)
			{
				Member* memVassal = oldAllegiance->FindMember(member->m_dwVassals[iVas]);
				newAllegiance->members.insert(MemberList::value_type(memVassal->m_dwGUID, (*memVassal)));
				
				UpdatePlayerAllegID(memVassal, newAllegianceID);

				SwitchNonMonarchVassals(memVassal, oldAllegianceID, newAllegianceID);// recurse to this function
							
				oldAllegiance->members.erase(oldAllegiance->members.find(memVassal->m_dwGUID));	// erase the member from the old allegiance
			}
		}

		return true;
	}

	return false;
}

/**
 *	Adds a vassal to the patron's vassal list.
 *
 *	The patron's follower count and rank are both recaluated.
 *
 *	@param memPatron - A pointer to the member record of the patron.
 *	@param memVassal - A pointer to the member record of the vassal to add to his/her patron's vassal list.
 */
bool cAllegiance::AddDirectVassal(Member* memPatron, Member* memVassal)
{
	if (memPatron)
	{
		for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
		{
			if (memPatron->m_dwVassals[iVas] == 0)
			{
				memPatron->m_dwVassals[iVas] = memVassal->m_dwGUID;
				UpdateFollowers(memPatron, memVassal->m_dwFollowers + 1);
				UpdateRank(memPatron);

				return true;
			}
		}
	}

	return false;
}

/**
 *	Adds a vassal to the patron's vassal list.
 *
 *	The patron's follower count and rank are both recaluated.
 *
 *	@param memPatron - The GUID of the patron.
 *	@param memVassal - The GUID of the vassal to add to his/her patron's vassal list.
 */
bool cAllegiance::AddDirectVassal(DWORD dwPatronGUID, DWORD dwVassalGUID)
{
	Member* memPatron = FindMember(dwPatronGUID);
	Member* memVassal = FindMember(dwVassalGUID);
	
	if (memPatron)
	{
		for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
		{
			if (memPatron->m_dwVassals[iVas] == 0)
			{
				memPatron->m_dwVassals[iVas] = dwVassalGUID;
				UpdateFollowers(memPatron, memVassal->m_dwFollowers + 1);
				UpdateRank(memPatron);

				return true;
			}
		}
	}

	return false;
}

/**
 *	Removes a vassal from the patron's vassal list.
 *
 *	The patron's follower count and rank are both recaluated.
 *
 *	@param memVassal - A pointer to the member record of the vassal to remove from his/her patron's vassal list.
 */
bool cAllegiance::RemDirectVassal(Member* memVassal)
{
	Member* memPatron = FindMember(memVassal->m_dwPatron);

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (memPatron->m_dwVassals[iVas] == memVassal->m_dwGUID)
		{
			memPatron->m_dwVassals[iVas] = 0;
			UpdateFollowers(memPatron, -(memVassal->m_dwFollowers + 1));
			UpdateRank(memPatron);

			return true;
		}
	}

	return false;
}

/**
 *	Removes a vassal from the patron's vassal list.
 *
 *	The patron's follower count and rank are both recaluated.
 *
 *	@param memVassal - The GUID of the vassal to remove from his/her patron's vassal list.
 */
bool cAllegiance::RemDirectVassal(DWORD dwVassalGUID)
{
	Member* memVassal = FindMember(dwVassalGUID);
	Member* memPatron = FindMember(memVassal->m_dwPatron);

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (memPatron->m_dwVassals[iVas] == memVassal->m_dwGUID)
		{
			memPatron->m_dwVassals[iVas] = 0;
			UpdateFollowers(memPatron, -(memVassal->m_dwFollowers + 1));
			UpdateRank(memPatron);

			return true;
		}
	}

	return false;
}

/**
 *	Caclulates the number of direct vassals sworn to a given character.
 *
 *	@param memPatron - A pointer to the member record of the member whose direct vassals should be calculated.
 *
 *	@return int - The number of direct vassals.
 */
int cAllegiance::GetVassalCount(Member* memPatron)
{
	int vassalCount = 0;

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (memPatron->m_dwVassals[iVas] != 0)
		{
			vassalCount++;
		}
	}

	return vassalCount;
}

/**
 *	Calculates the number of direct vassals sworn to a given character.
 *
 *	@param dwPatronGUID - The GUID of the member whose direct vassals should be calculated.
 *
 *	@return int - The number of direct vassals.
 */
int cAllegiance::GetVassalCount(DWORD dwPatronGUID)
{
	int vassalCount = 0;

	Member* memPatron = FindMember(dwPatronGUID);

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (memPatron->m_dwVassals[iVas] != 0)
		{
			vassalCount++;
		}
	}

	return vassalCount;
}

/**
 *	Calculates the number of followers under a given character.
 *
 *	@param memPatron - A pointer to the member record of the member whose followers should be calculated.
 *
 *	@return int - The number of followers.
 */
int cAllegiance::GetFollowerCount(Member* member)
{
	int followerCount = 0;

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (member->m_dwVassals[iVas] != 0)
		{
			followerCount += 1;	// add the vassal to the follower count

			Member* memVassal = FindMember(member->m_dwVassals[iVas]);
			followerCount += GetFollowerCount(memVassal);	//add the vassal's followers to the follower count
		}
	}

	return followerCount;
}

/**
 *	Calculates the number of followers under a given character.
 *
 *	@param dwPatronGUID - The GUID of the member whose followers should be calculated.
 *
 *	@return int - The number of followers.
 */
int cAllegiance::GetFollowerCount(DWORD dwMemberGUID)
{
	int followerCount = 0;

	Member* member = FindMember(dwMemberGUID);

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (member->m_dwVassals[iVas] != 0)
		{
			followerCount += 1;	// add the vassal to the follower count

			Member* memVassal = FindMember(member->m_dwVassals[iVas]);
			followerCount += GetFollowerCount(memVassal->m_dwGUID);//add the vassal's followers to the follower count
		}
	}

	return followerCount;
}

/**
 *	Updates the allegiance ID for a player.
 *
 *	The function is used to update a player's allegiance ID when he/she joins an allegiance or his/her allegiance is switched.
 *
 *	@param member - A pointer to the member record of the player whose allegiance ID should be changed.
 *	@param newAllegianceID - The player's new allegiance ID.
 */
void cAllegiance::UpdatePlayerAllegID(Member* member, DWORD newAllegianceID)
{
	if (member)
	{
		// update each logged-in avatar's allegiance ID in memory
		cAvatar* aAvatar = cClient::FindAvatar(member->m_dwGUID);
		if (aAvatar)
			aAvatar->SetAllegianceID(newAllegianceID);

		// update each avatar's allegiance ID in the database
		UpdateAvatarAllegIDDB(member->m_dwGUID, newAllegianceID);

		if (newAllegianceID == 0)
			DeleteMemberRecordDB(newAllegianceID, member->m_dwGUID);
		else
			UpdateMemberAllegIDDB(member->m_dwGUID, newAllegianceID);
	}
}

/**
 *	Updates the allegiance ID for a player.
 *
 *	The function is used to update a player's allegiance ID when he/she joins an allegiance or his/her allegiance is switched.
 *
 *	@param dwPlayerGUID - The GUID of the player whose allegiance ID should be changed.
 *	@param newAllegianceID - The player's new allegiance ID.
 */
void cAllegiance::UpdatePlayerAllegID(DWORD dwPlayerGUID, DWORD newAllegianceID)
{
	// update each logged-in avatar's allegiance ID in memory
	cAvatar* aAvatar = cClient::FindAvatar(dwPlayerGUID);
	if (aAvatar)
		aAvatar->SetAllegianceID(newAllegianceID);

	// update each avatar's allegiance ID in the database
	UpdateAvatarAllegIDDB(dwPlayerGUID, newAllegianceID);

	if (newAllegianceID == 0)
			DeleteMemberRecordDB(newAllegianceID, dwPlayerGUID);
		else
			UpdateMemberAllegIDDB(dwPlayerGUID, newAllegianceID);
}

/**
 *	Updates the follower count for a branch of the allegiance tree.
 *
 *	The function is used to update the tree when a member is added or removed.
 *	Each follower of that member must be added/removed to/from the follower count for each allegiance member higher in the tree.
 *
 *	@param member - A pointer to the member record of the base member from whom to start.
 *	@param numFollowers - The number of followers added or removed.
 */
void cAllegiance::UpdateFollowers(Member* member, int numFollowers)
{
	if (member)
	{
		member->m_dwFollowers += numFollowers;

		// update the allegiance member database record
		UpdateMemberFollowersDB(member->m_dwGUID, member->m_dwFollowers);

		if (member->m_dwGUID != m_LeaderGUID)
		{
			Member* memPatron = FindMember(member->m_dwPatron);
			UpdateFollowers(memPatron, numFollowers);
		}
	}
}

/**
 *	Updates the follower count for a branch of the allegiance tree.
 *
 *	The function is used to update the tree when a member is added or removed.
 *	Each follower of that member must be added/removed to/from the follower count for each allegiance member higher in the tree.
 *
 *	@param memberGUID - The GUID of the base member from whom to start.
 *	@param numFollowers - The number of followers added or removed.
 */
void cAllegiance::UpdateFollowers(DWORD dwMemberGUID, int numFollowers)
{
	Member* member = FindMember(dwMemberGUID);

	if (member)
	{
		member->m_dwFollowers += numFollowers;

		// update the allegiance member database record
		UpdateMemberFollowersDB(member->m_dwGUID, member->m_dwFollowers);

		if (member->m_dwGUID != m_LeaderGUID)
		{
			UpdateFollowers(member->m_dwPatron, numFollowers);
		}
	}
}

/**
 *	Updates the rank of a member.
 *
 *	A patron's rank is equal to the rank of the highest vassal or one higher than the two highest vassals of equal rank, whichever is greater.
 *	A patron's rank may change if he/she gains/loses a vassal, or if a vassal's rank changes.
 *	If a member's rank has changed, his/her patron's rank is also recalculated.
 *	The minimum rank is 1.
 *
 *	@param member - A pointer to the member record of the base member/patron from whom to start.
 */
void cAllegiance::UpdateRank(Member* member)
{
	int newRank = 1;
	int rankCount = 0;

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (member->m_dwVassals[iVas] != 0)
		{
			Member* memVassal = FindMember(member->m_dwVassals[iVas]);
			if (memVassal->m_bRank > newRank)
			{
				newRank = memVassal->m_bRank;
				rankCount = 1;
			}
			else if (memVassal->m_bRank == newRank)
			{
				rankCount++;
			}
		}
	}

	if (rankCount >= 2)	// if two vassals have the same rank
	{
		//add one to the rank
		newRank = newRank + 1;
	}

	if (newRank > MAX_RANK)
	{
		newRank = MAX_RANK;
	}

	if (newRank != member->m_bRank)	// if the new rank is different from the old rank
	{
		// update the member's rank, and the rank of his/her patron
		member->m_bRank = newRank;

		// update the allegiance member database record
		UpdateMemberRankDB(member->m_dwGUID, member->m_bRank);

		if (member->m_dwGUID != this->GetLeader())
			UpdateRank(member->m_dwPatron);
	}
}

/**
 *	Updates the rank of a member.
 *  
 *	A patron's rank is equal to the rank of the highest vassal or one higher than the two highest vassals of equal rank, whichever is greater.
 *	A patron's rank may change if he/she gains/loses a vassal, or if a vassal's rank changes.
 *	If a member's rank has changed, his/her patron's rank is also recalculated.
 *	The minimum rank is 1.
 *
 *	@param memberGUID - The GUID of the base member/patron from whom to start.
 */
void cAllegiance::UpdateRank(DWORD dwMemberGUID)
{
	Member* member = FindMember(dwMemberGUID);

	int newRank = 1;
	int rankCount = 0;

	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (member->m_dwVassals[iVas] != 0)
		{
			Member* memVassal = FindMember(member->m_dwVassals[iVas]);
			if (memVassal->m_bRank > newRank)
			{
				newRank = memVassal->m_bRank;
				rankCount = 1;
			}
			else if (memVassal->m_bRank == newRank)
			{
				rankCount++;
			}
		}
	}

	if (rankCount >= 2)	// if two vassals have the same rank
	{
		//add one to the rank
		newRank = newRank + 1;
	}

	if (newRank > MAX_RANK)
	{
		newRank = MAX_RANK;
	}

	if (newRank != member->m_bRank)	// if the new rank is different from the old rank
	{	// update the member's rank, and the rank of his/her patron
		member->m_bRank = newRank;
		
		// update the allegiance member database record
		UpdateMemberRankDB(member->m_dwGUID, member->m_bRank);

		if (member->m_dwGUID != this->GetLeader())
			UpdateRank(member->m_dwPatron);
	}
}

/*
				Allegiance Experience Pass-up

"OLD" FORMULA:
(Pre "The Madness of Men" event - February 24, 2004)

	The XP pass-up formula based upon Zerot's Experience Calculator.

	The pass-up XP from vassal to patron is calculated as follows:

	Generated % - % XP to patron of vassal's earned (hunting) XP.
	Received % - % XP that patron will receive from his vassal's generated XP.
	Passup % - % XP actually received by patron from vassal's earned (hunting) XP.

	Generated % = 8.9 + (2 * 3.5 * (ALoy / 100))
	Received % = 68 + (.35 * ALdr) (max 1.25)
	Passup % = Generated % * Received % / 100.0

	where,
		ALoy = Adjusted Loyalty = BLoy + ((IG / 240) * BLoy)
		BLoy = Buffed Loyalty
		IG = actual in-game time sworn to patron in hours (720 max)
		ALdr = Adjusted Leadership (163 max) = BLrd + (((7.5 * V) / 100) * BLrd) + ((IG2 / 1200) * BLrd)
		BLrd = Buffed Leadership
		V = vassal factor ((7.5 * vassals) / 100) (.90 max)
		IG2 = average in-game time sworn to patron for all vassals in hours (240 max)

	---------------------

	The pass-up from grand vassal to grand patron is calculated as follows:

	Generated2 % - % XP to patron of vassal's pass-up (grand vassal) XP.
	Received2 % - % XP that patron will receive from his vassal's generated XP.
	Passup2 % - % XP actually received by patron from vassal's pass-up (grand vassal) XP.

	Generated2 % = 45 + (.15 * ALoy)
	Received2 % = 68 + (.35 * ALdr) (max 1.25)
	Passup2 % = Generated2 % * Received2 % / 100.0

	where,
		ALoy = Adjusted Loyalty = BLoy + ((IG / 240) * BLoy)
		BLoy = Buffed Loyalty
		IG = actual in-game time sworn to patron in hours (720 max)
		ALdr = Adjusted Leadership (163 max) = BLrd + (((7.5 * V) / 100) * BLrd) + ((IG2 / 1200) * BLrd)
		BLrd = Buffed Leadership
		V = vassal factor ((7.5 * vassals) / 100) (.90 max)
		IG2 = average in-game time sworn to patron for all vassals in hours (240 max)

------------------------------------------

"NEW" FORMULA:
(Post "The Madness of Men" event - February 24, 2004)

	The XP pass-up formula of Xerxes of Thistledown.

	The pass-up XP from vassal to patron is calculated as follows:
	NOTE: Extensive testing has been done over a period of 4 months to verify these results.

	Generated % - % XP to patron of vassal's earned (hunting) XP.
	Received % - % XP that patron will receive from his vassal's generated XP.
	Passup % - % XP actually received by patron from vassal's earned (hunting) XP.

	Generated % = 50.0 + 22.5 * (BLoy / 291) * (1.0 + (RT/730) * (IG/720))
	Received % = 50.0 + 22.5 * (BLdr / 291) * (1.0 + V * (RT2/730) * (IG2/720))
	Passup % = Generated % * Received % / 100.0

	where,
		BLoy = Buffed Loyalty (291 max)
		BLdr = Buffed Leadership (291 max)
		RT = actual real time sworn to patron in days (730 max)
		IG = actual in-game time sworn to patron in hours (720 max)
		RT2 = average real time sworn to patron for all vassals in days (730 max)
		IG2 = average in-game time sworn to patron for all vassals in hours (720 max)
		V = vassal factor (1 = 0.25, 2 = 0.50, 3 = 0.75, 4+ = 1.00) (1.0 max)

	The Generated % is what is reported on the allegiance panel.

	---------------------

	The pass-up from grand vassal to grand patron is calculated as follows:
	NOTE: While, I have done very little testing on grand vassal XP Passup, I would speculate that the equation is similar to vassal Passup:

	Generated2 % - % XP to patron of vassal's pass-up (grand vassal) XP.
	Received2 % - % XP that patron will receive from his vassal's generated XP.
	Passup2 % - % XP actually received by patron from vassal's pass-up (grand vassal) XP.

	Generated2 % = 16.0 + 8.0 * (BLoy / 291) * (1.0 + (RT/730) * (IG/720))
	Received2 % = 16.0 + 8.0 * (BLdr / 291) * (1.0 + V * (RT2/730) * (IG2/720))
	Passup2 % = Generated2 % * Received2 % / 100.0

	where,
		BLoy = Buffed Loyalty (291 max)
		BLdr = Buffed Leadership (291 max)
		RT = actual real time sworn to patron in days (730 max)
		IG = actual in-game time sworn to patron in hours (720 max)
		RT2 = average real time sworn to patron for all vassals in days (730 max)
		IG2 = average in-game time sworn to patron for all vassals in hours (720 max)
		V = vassal factor (1 = 0.25, 2 = 0.50, 3 = 0.75, 4+ = 1.00) (1.0 max)

	Passup % * Passup2 % / 100.0 would be the %-tage the grand patron receives from a grand vassal's earned (hunting) XP.
*/

/**
 *	Calculates the percentage of experience passed up from a vassal.
 *
 *	@param memVassal - A pointer to the member record of the vassal to use for the calculations.
 *	@param experience - The amount of experience from which to calculate pass-up.
 *	@param isGrandVassal - Is a grand vassal (a non-originator of the experience).
 */
void cAllegiance::VassalPassupXP(Member* memVassal, UINT64 experience, bool isGrandVassal)
{
	// find the patron's allegiance record
	if (memVassal->m_dwPatron != NULL)
	{
		Member* memPatron = FindMember(memVassal->m_dwPatron);
		
		UINT64 generatedXP, receivedXP;

		generatedXP = PASSED_XP_MULT * CalcVassalGeneratedXP(memVassal, experience, isGrandVassal);
		UpdateGeneratedXP(memVassal,generatedXP);
		
		receivedXP = RECEIVED_XP_MULT * CalcPatronReceivedXP(memPatron, generatedXP, isGrandVassal);
		UpdateEarnedXP(memPatron,receivedXP);

		if (receivedXP > 0)
		{
			if (memPatron->m_dwGUID != this->m_LeaderGUID)	// if not the monarch
			{
				VassalPassupXP(memPatron, receivedXP, true);// further experience pass-up; recurse to this function
			}
		}
	}
}

/**
 *	Updates the member's generated experience.
 *
 *
 *	@param member - A pointer to the member record of the member whose generated experience should be updated.
 *	@param experience - The amount of experience generated.
 */
void cAllegiance::UpdateGeneratedXP(Member* member, UINT64 experience)
{
	if (experience > 0)
	{
		member->m_passupXP += experience;

		// update the allegiance member database record
		UpdateMemberXPPassupDB(member->m_dwGUID, member->m_passupXP);
	}
}

/**
 *	Updates the member's earned experience.
 *
 *	The member may be logged-on or logged-off:
 *		If the member is logged-on, the experience is added directly to the avatar's unassigned experience.
 *		If the member is logged-off, the experience is added to the avatar's pending experience.
 *
 *	@param member - A pointer to the member record of the member whose experience should be updated.
 *	@param experience - The amount of experience earned.
 */
void cAllegiance::UpdateEarnedXP(Member* member, UINT64 experience)
{
	if (experience > 0)
	{
		if (member->m_online)
		{
			cClient *pcClient = cClient::FindClient( member->m_dwGUID );
			if (pcClient)
			{
				pcClient->m_pcAvatar->UpdateUnassignedExp(experience);
			}	
		}
		else
		{
			member->m_pendingXP += experience;

			// update the allegiance member database record
			UpdateMemberXPPendingDB(member->m_dwGUID, member->m_pendingXP);
		}
	}
}

/**
 *	Calculates the percentage of experience passed up from a vassal.
 *
 *	Generated % - % XP to patron of vassal's Earned (hunting) XP
 *
 *	Old Vassal Generated % = 8.9 + (2 * 3.5 * (ALoy / 100))
 *	New Vassal Generated % = 50.0 + 22.5 * (BLoy / 291) * (1.0 + (RT/730) * (IG/720))
 *	Old Grand Vassal Generated % = 45 + (.15 * ALoy)
 *	New Grand Vassal Generated % = 16.0 + 8.0 * (BLoy / 291) * (1.0 + (RT/730) * (IG/720))
 *
 *	Variables:
 *		ALoy = Adjusted Loyalty = BLoy + ((IG / 240) * BLoy)
 *		BLoy = Buffed Loyalty (291 max)
 *		RT = actual real-time sworn to patron in days (730 max)
 *		IG = actual in-game time sworn to patron in hours (720 max)
 *
 *	@param memVassal - A pointer to the member record of the vassal to use for the calculations.
 *
 *	@return UINT - The amount of experience generated.
 */
UINT64 cAllegiance::CalcVassalGeneratedXP(Member* memVassal, UINT64 passedXP, bool isGrandVassal)
{
	time_t curtime;
	curtime = time (NULL);	// the current time

	int bufLoyalty = memVassal->m_wLoyalty;
	float igSwornTimeDays = (memVassal->m_timeIGTotal - memVassal->m_timeIGSwear) / 86400;

	if (OLD_PASSUP)
	{
		// units are in days: 240 hours = 10 days
		if (igSwornTimeDays > 10)	igSwornTimeDays = 10;
		int aLoyalty = bufLoyalty + ((igSwornTimeDays / 10) * bufLoyalty);

		if (isGrandVassal)
		{
			if (aLoyalty > 200)		aLoyalty = 200;
			
			return (INT64)passedXP * (float)((45 + (.15 * aLoyalty)) / 100);
		}
		else
		{
			return (INT64)passedXP * (float)((8.9 + (2 * 3.5 * (aLoyalty / 100))) / 100);
		}
	}
	else
	{
		float rtSwornTimeDays = (curtime - memVassal->m_timeRLSwear) / 86400;

		if (rtSwornTimeDays > 730)	rtSwornTimeDays = 730;
		if (igSwornTimeDays > 720)	igSwornTimeDays = 720;

		if (isGrandVassal)
			return (INT64)passedXP * (float)((16.0 + 8.0 * (bufLoyalty / 291) * (1.0 + (rtSwornTimeDays/730) * (igSwornTimeDays/720))) / 100.0);
		else
			return (INT64)passedXP * (float)((50.0 + 22.5 * (bufLoyalty / 291) * (1.0 + (rtSwornTimeDays/730) * (igSwornTimeDays/720))) / 100.0);
	}
}

/**
 *	Calculates the percentage of experience received by a grand patron.
 *
 *	Received % - % XP that patron will receive from his vassal's Generated XP.
 *
 *	Old Patron Received % = 68 + (.35 * ALdr) (max 1.25)
 *	New Patron Received % = 50.0 + 22.5 * (BLdr / 291) * (1.0 + V * (RT2/730) * (IG2/720))
 *	Old Grand Patron Received % = 68 + (.35 * ALdr) (max 1.25)
 *	New Grand Patron Received % = 16.0 + 8.0 * (BLdr / 291) * (1.0 + V * (RT2/730) * (IG2/720))
 *	
 *	Variables:
 *		ALdr = Adjusted Leadership (163 max) = BLrd + (((7.5 * V) / 100) * BLrd) + ((IG2 / 1200) * BLrd)
 *		BLdr = Buffed Leadership (291 max)
 *		RT2 = average real-time sworn to patron for all vassals in days (730 max)
 *		IG2 = average in-game time sworn to patron for all vassals in hours (720 max)
 *		V = vassal factor
 *			Old: number of vassals
 *			New: (1 = 0.25, 2 = 0.50, 3 = 0.75, 4+ = 1.00) (1.0 max)
 *
 *	@param memPatron - A pointer to the member record of the patron to use for the calculations.
 *
 *	@return UIN - The amount of experience received.
 */
UINT64 cAllegiance::CalcPatronReceivedXP(Member* memPatron, UINT64 receivedXP, bool isGrandPatron)
{
	int bufLeadership = memPatron->m_wLeadership;
	float avgIGSwornTime = memPatron->m_avgRTSworn / 86400;
	float vassalFactor = CalcVassalFactor(GetVassalCount(memPatron));
	
	if (OLD_PASSUP)
	{
		if (avgIGSwornTime > 240)	avgIGSwornTime = 240;
		int aLeadership = bufLeadership + (((7.5 * vassalFactor) / 100) * bufLeadership) + ((avgIGSwornTime / 1200) * bufLeadership);

		float receivedPercent = (68 + (.35 * aLeadership)) / 100;
		if (receivedPercent > 1.25)
			receivedPercent = 1.25;

		return ((INT64)receivedXP * receivedPercent);
	}
	else
	{
		float avgRTSwornTime = memPatron->m_avgIGSworn / 86400;

		if (avgRTSwornTime > 730)	avgRTSwornTime = 730;
		if (avgIGSwornTime > 720)	avgIGSwornTime = 720;

		if(isGrandPatron)
			return (INT64)receivedXP * (float)((16.0 + 8.0 * (bufLeadership / 291) * (1.0 + vassalFactor * (avgRTSwornTime/730) * (avgIGSwornTime/720))) / 100.0);
		else
			return (INT64)receivedXP * (float)((50.0 + 22.5 * (bufLeadership / 291) * (1.0 + vassalFactor * (avgRTSwornTime/730) * (avgIGSwornTime/720))) / 100.0);
	}
}

/**
 *	Calculates the average sworn time for a patron's vassals and updates the patron's member record.
 *
 *	This function calculates both the average in-game and average real-time sworn time for a patron's vassals.
 *	These values are used in the calculation of vassal experience pass-up to a patron.
 *
 *	@param memPatron - A pointer to the member record of the patron to use for the calculations.
 */
void cAllegiance::UpdateAverageSwornTimeOfVassals(Member* memPatron)
{	
	time_t curtime;
	curtime = time (NULL);	//the current time

	int numVassals = 0;
	float totalSwornTimeIG = 0;
	float totalSwornTimeRT = 0;

	// find each vassal's allegiance record
	for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
	{
		if (memPatron->m_dwVassals[iVas] != 0)
		{
			Member* memVassal = FindMember(memPatron->m_dwVassals[iVas]);
			totalSwornTimeIG += memVassal->m_timeIGTotal - memVassal->m_timeIGSwear;
			totalSwornTimeRT += curtime - memVassal->m_timeRLSwear;
			numVassals++;
		}
	}

	memPatron->m_avgIGSworn = totalSwornTimeIG / numVassals;	// average in-game sworn time of vassals
	memPatron->m_avgRTSworn = totalSwornTimeRT / numVassals;	// average real-time sworn time of vassals
}

/**
 *	Calculates the vassal factor based upon a patron's number of vassals.
 *
 *	Used in the calculation of patron-received experience.
 *
 *	@param numVassals - The number of vassals sworn to a given patron.
 *
 *	@return float - The vassal factor.
 */
float cAllegiance::CalcVassalFactor(int numVassals)
{	
	if (OLD_PASSUP)
	{
		return numVassals;
	}
	else
	{
		if (numVassals >= 4)
			return 1;
		else if (numVassals == 3)
			return .75;
		else if (numVassals == 2)
			return .50;
		else if (numVassals == 1)
			return .25;
		else
			return 0;
	}
}


/* Allegiance Database Updates */
void cAllegiance::Disband()
{
	std::map<DWORD, Member>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		// update logged-in avatar's allegiance ID in memory
		cAvatar* aAvatar = cClient::FindAvatar(itrMember->first);
		if (aAvatar)
			aAvatar->m_dwAllegianceID = 0;

		// update each avatar's allegiance ID in the database
		UpdateAvatarAllegIDDB(itrMember->first, 0);
	}

	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "DELETE FROM `allegiance` WHERE AllegianceID = %lu;",m_ID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	sprintf( szCommand, "DELETE FROM `allegiance_members` WHERE AllegianceID = %lu;",m_ID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	members.clear();
	delete this;
}

void cAllegiance::UpdateAvatarAllegIDDB(DWORD dwGUID, DWORD allegianceID)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `avatar` SET `AllegianceID` = %lu WHERE AvatarGUID = %lu;",allegianceID, dwGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberAllegIDDB(DWORD dwGUID, DWORD allegianceID)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `AllegianceID` = %lu WHERE MemberGUID = %lu;",allegianceID, dwGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateAvatarRecordDB(DWORD dwAvatarGUID)
{
	cAvatar* pcAvatar = cClient::FindAvatar(dwAvatarGUID);

	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `Loyalty` = %d, `Leadership` = %d WHERE MemberGUID = %lu;",pcAvatar->GetTotalLoyalty(),pcAvatar->GetTotalLeadership(),pcAvatar->GetGUID());
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::AddMemberRecordDB(DWORD allegianceID, DWORD dwMemberGUID, DWORD dwPatronGUID)
{
	cAllegiance* aAllegiance = GetAllegianceByID(allegianceID);
	Member* member = aAllegiance->FindMember(dwMemberGUID);

	char		szCommand[1024];
	RETCODE		retcode;

	sprintf( szCommand, "INSERT INTO `allegiance_members` ( `AllegianceID`, `MemberGUID`, `MemberName`, `Gender`, `Race`, `Rank`, `Loyalty`, `Leadership`, `XP_Passup`, `XP_Pending`, `Patron`, `Followers`, `TimeSwornRL`, `TimeSwornIG` ) VALUES (%lu, %lu, '%s', %d, %d, %d, %d, %d, %lu, %lu, %lu, %d, %lu, %lu);",allegianceID, member->m_dwGUID, member->m_szName.c_str(), member->m_bGender, member->m_bRace, member->m_bRank, member->m_wLoyalty, member->m_wLeadership, 0, 0, dwPatronGUID, 0, time (NULL), 0);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::DeleteMemberRecordDB(DWORD allegianceID, DWORD dwMemberGUID)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "DELETE FROM `allegiance_members` WHERE MemberGUID = %lu AND AllegianceID = %lu;",dwMemberGUID, allegianceID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberRecordDB(Member* member)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `XP_Passup` = %lu, `XP_Pending` = %lu WHERE MemberGUID = %lu;",member->m_passupXP,member->m_pendingXP,member->m_dwGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberXPPassupDB(DWORD memberGUID, UINT64 passedXP)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `XP_Passup` = %lu WHERE MemberGUID = %lu;",(DWORD)passedXP, memberGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberXPPendingDB(DWORD memberGUID, UINT64 pendingXP)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `XP_Pending` = %lu WHERE MemberGUID = %lu;",(DWORD)pendingXP, memberGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberFollowersDB(DWORD memberGUID, int numFollowers)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `Followers` = %d WHERE MemberGUID = %lu;", numFollowers, memberGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberRankDB(DWORD memberGUID, int rank)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `Rank` = %d WHERE MemberGUID = %lu;", rank, memberGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

void cAllegiance::UpdateMemberPatronDB(DWORD memberGUID, DWORD patronGUID)
{
	char		szCommand[512];
	RETCODE		retcode;

	sprintf( szCommand, "UPDATE `allegiance_members` SET `Patron` = %lu WHERE MemberGUID = %lu;", patronGUID, memberGUID);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}