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
 *	@file Fellowship.cpp
 *	Implements general functionality for fellowships.
 *
 *	TODO:	Separate static and non-static (instance-based) functions.
 *			(Static functions should probably be in a fellowship manager file.)
 *			Make fellowship stat update messages active (sent periodically) rather than passive (sent for every change).
 */

#pragma warning(disable:4786)	//warning: identifier was truncated to '255' characters in the browser information

#include "Client.h"
#include "Fellowship.h"
#include "MasterServer.h"

cFellowship::cFellowship()
{
    m_ID = 0;
    m_Name = "";
    m_LeaderGUID = 0;
	m_Size = 0;
	m_CreationTime = 0;
	m_IsOpen = false;
	m_ShareXP = true;
	m_ShareLoot = false;
	m_ProportionXP = false;
}

cFellowship::~cFellowship()
{

}

DWORD cFellowship::NewFellowship(DWORD dwLeaderGUID, std::string name, char felName[50], bool shareXP, bool shareLoot)
{
	cFellowship* newFellowship = new cFellowship();
	return newFellowship->CreateFellowship(dwLeaderGUID, name, felName, shareXP, shareLoot);
}

DWORD cFellowship::CreateFellowship(DWORD dwLeaderGUID, std::string name, char felName[50], bool shareXP, bool shareLoot)
{
	m_ID = cMasterServer::NewFellowID();
	m_LeaderGUID = dwLeaderGUID;
    m_Name = felName;
	m_Size = 0;
	m_CreationTime = time(NULL);
	m_IsOpen = false;
	m_ShareXP = shareXP;
	m_ShareLoot = shareLoot;
	m_ProportionXP = false;

	cMasterServer::m_FellowshipList.push_back( this );

	AddMember(dwLeaderGUID);

	return m_ID;
}

cFellowship* cFellowship::GetFellowshipByID(DWORD dwFellowshipID)
{
	for ( std::vector<cFellowship *>::iterator itr = cMasterServer::m_FellowshipList.begin() ; itr != cMasterServer::m_FellowshipList.end() ; ++itr )
	{
		if ((*itr)->GetID() == dwFellowshipID)
        {
            return *itr;
        }
	}
	
	return NULL;
}

void cFellowship::Disband()
{
	ClearMembers();

	delete this;
}

/**
 *	Clears all members from the fellowship.
 */
void cFellowship::ClearMembers()
{
	cClient*	pcClient;
	cAvatar*	pcAvatar;
	// vector of fellowship member count (1 per member)
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
		if (pcClient)
		{
			pcAvatar = pcClient->m_pcAvatar;
			if (pcAvatar)
			{
				pcAvatar->SetFellowshipID(0);
				pcAvatar->inFellow = false;

				cMessage cmRemove = DisbandMessage(itrMember->second.m_dwGUID, ++pcClient->m_dwF7B0Sequence);
				pcClient->AddPacket( WORLD_SERVER, cmRemove, 4 );
			}
		}
	}

	members.clear();
}

/**
 *	Handles the opening or closing of the fellowship.
 *
 *	@param dwIsOpen - Value for whether the fellowship should be opened or closed
 */
bool cFellowship::SetOpenClose(DWORD dwIsOpen)
{
	cMessage	cmOpenCloseText;
	
	char		szTextBuffer[255];
	DWORD		dwColor = 0x17L;

	if (dwIsOpen == 1)
	{
		sprintf(&szTextBuffer[0],"%s is now an open fellowship; anyone may recruit new members.", this->GetName().c_str());
		this->m_IsOpen = true;
	}
	else
	{
		sprintf(&szTextBuffer[0],"%s is now a closed fellowship.", this->GetName().c_str());
		this->m_IsOpen = false;
	}

	cmOpenCloseText << 0xF62CL << szTextBuffer << dwColor;

	cClient*	pcClient;
	cAvatar*	pcAvatar;
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
		if (pcClient)
		{
			pcAvatar = pcClient->m_pcAvatar;
			if (pcAvatar)
			{
				pcClient->AddPacket( WORLD_SERVER, cmOpenCloseText, 4 );
				cMessage cmOpenCloseMessage = JoinMessage( pcAvatar->GetGUID(), ++pcClient->m_dwF7B0Sequence );
				pcClient->AddPacket( WORLD_SERVER, cmOpenCloseMessage, 4 );
			}
		}
	}

	return true;
}

/**
 *	Handles the promotion of a member of the fellowship to leader.
 *
 *	@param dwNewLeaderGUID - The GUID for the member that is being promoted to leader.
 */
bool cFellowship::SetLeader(DWORD dwNewLeaderGUID)
{
	cMessage	cmNewLeaderText;
	
	char		szTextBuffer[255];
	DWORD		dwColor = 0x04L;

	FellowMem* member = this->FindMember(dwNewLeaderGUID);

	if (member)
	{
		this->m_LeaderGUID = 2;

		sprintf(&szTextBuffer[0],"%s is now the leader of this fellowship.", member->m_szName.c_str());

		cmNewLeaderText << 0xF62CL << szTextBuffer << dwColor;

		cClient*	pcClient;
		cAvatar*	pcAvatar;
		std::map<DWORD, FellowMem>::iterator itrMember;
		for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
		{
			pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
			if (pcClient)
			{
				pcAvatar = pcClient->m_pcAvatar;
				if (pcAvatar)
				{
					cMessage cmNewLeaderMessage = JoinMessage( pcAvatar->GetGUID(), ++pcClient->m_dwF7B0Sequence );
					pcClient->AddPacket( WORLD_SERVER, cmNewLeaderMessage, 4 );
					pcClient->AddPacket( WORLD_SERVER, cmNewLeaderText, 4 );
				}
			}
		}
		return true;
	}

	return false;
}

/**
 *	Handles the addition of a member of the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member that is being added to the fellowship.
 */
bool cFellowship::AddMember(DWORD dwAvatarGUID)
{
	this->InsertMember(dwAvatarGUID);

	return true;
}

/**
 *	Handles the removal of a member of the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member that quit the fellowship.
 */
bool cFellowship::RemMember(DWORD dwMemberGUID)
{
	FellowMem* member = this->FindMember(dwMemberGUID);

	if (member)
	{
		if (member->m_dwGUID == this->m_LeaderGUID)
		{
			this->Disband();
		}
		else
		{
			DeleteMember(dwMemberGUID, false);
		}

		return true;
	}

	return false;
}

/**
 *	Handles the dismissal of a member from the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member that was dismissed.
 */
bool cFellowship::DismissMember(DWORD dwMemberGUID)
{
	DeleteMember(dwMemberGUID, true);

	return true;
}

/**
 *	Handles the death of a member of the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member that died.
 */
bool cFellowship::MemberDeath(DWORD dwMemberGUID)
{
	FellowMem* member = this->FindMember(dwMemberGUID);

	if (member)
	{
		cMessage	cmMemberDeathText;
	
		char		szTextBuffer[255];
		DWORD		dwColor = 0x17L;

		sprintf(&szTextBuffer[0],"Your fellow %s has died!", member->m_szName.c_str());

		cmMemberDeathText << 0xF62CL << szTextBuffer << dwColor;

		cClient*	pcClient;
		cAvatar*	pcAvatar;
		std::map<DWORD, FellowMem>::iterator itrMember;
		for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
		{
			pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
			if (pcClient && itrMember->second.m_dwGUID != dwMemberGUID)
			{
				pcAvatar = pcClient->m_pcAvatar;
				if (pcAvatar)
				{
					pcClient->AddPacket( WORLD_SERVER, cmMemberDeathText, 4 );
					cMessage cmUpdate = UpdMemberMessage(itrMember->second.m_dwGUID, ++pcClient->m_dwF7B0Sequence, dwMemberGUID);
					pcClient->AddPacket( WORLD_SERVER, cmUpdate, 4 );
				}
			}
		}

		return true;
	}

	return false;
}

/**
 *	Inserts an avatar into the fellowship member map.
 *
 *	The function is used when a player should be inserted into a fellowship.
 *
 *	@param dwAvatarGUID - The GUID for the avatar that should be inserted.
 */
bool cFellowship::InsertMember(DWORD dwAvatarGUID)
{
	FellowMem	newMember;
	cClient*	pcClient;
	cAvatar*	pcAvatar;

	pcClient = cClient::FindClient(dwAvatarGUID);
	pcAvatar = pcClient->m_pcAvatar;
	if (pcAvatar)
	{
		newMember.m_dwGUID = pcAvatar->GetGUID();
		newMember.m_wUnk1 = 0;
		newMember.m_dwLevel = pcAvatar->m_cStats.m_dwLevel;
		newMember.m_dwHealthTot = pcAvatar->GetTotalHealth();
		newMember.m_dwStaminaTot = pcAvatar->GetTotalStamina();
		newMember.m_dwManaTot = pcAvatar->GetTotalMana();
		newMember.m_dwHealthCur = pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent;
		newMember.m_dwStaminaCur = pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent;
		newMember.m_dwManaCur = pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent;
		(pcAvatar->m_dwOptions & 0x00100000) ? newMember.m_bShareLoot = 1 : newMember.m_bShareLoot = 0;
		newMember.m_szName = pcAvatar->Name();

		newMember.m_timeJoin = time(NULL);

		this->members.insert(MemberList::value_type(newMember.m_dwGUID, newMember));
		this->m_Size++;

		pcClient = cClient::FindClient(dwAvatarGUID);
		if (pcClient)
		{
			cMessage cmJoinMessage = JoinMessage( newMember.m_dwGUID, ++pcClient->m_dwF7B0Sequence );
			pcClient->AddPacket( WORLD_SERVER, cmJoinMessage, 4 );
		}
		if (this->m_Size > 1)
		{
			RelayMemberUpdate(newMember.m_dwGUID);
		}

		return true;
	}

	return false;
}

/**
 *	Removes an avatar from the fellowship member map.
 *
 *	The function is used when a player should be removed from a fellowship.
 *
 *	@param dwAvatarGUID - The GUID for the avatar that should be removed.
 *	@param wasDismissed - Value to determine whether the member was dismissed (or else quit).
 */
bool cFellowship::DeleteMember(DWORD dwMemberGUID, bool wasDismissed)
{
	this->members.erase(dwMemberGUID);		// erase the member from the fellowship
	this->m_Size--;							// update the fellowship size

	if (this->m_Size > 0)
	{
		if (wasDismissed)
			RelayMemberDismiss(dwMemberGUID);
		else
			RelayMemberDelete(dwMemberGUID);
	}

	return true;
}

/**
 *	Updates the information for a member of the fellowship in the member map.
 *
 *	The function is used whenever the fellowship information should be updated for a fellowship member.
 *
 *	@param dwMemberGUID - The GUID for the member whose information should be updated.
 */
bool cFellowship::UpdateMember(DWORD dwMemberGUID)
{
	FellowMem* member = this->FindMember(dwMemberGUID);

	if (member)
	{
		cClient*	pcClient;
		cAvatar*	pcAvatar;

		pcClient = cClient::FindClient(member->m_dwGUID);
		pcAvatar = pcClient->m_pcAvatar;
		if (pcAvatar)
		{
			if (member->m_dwLevel != pcAvatar->m_cStats.m_dwLevel)
			{
				CalcShareXP();
			}
			member->m_dwLevel = pcAvatar->m_cStats.m_dwLevel;
			member->m_dwHealthTot = pcAvatar->GetTotalHealth();
			member->m_dwStaminaTot = pcAvatar->GetTotalStamina();
			member->m_dwManaTot = pcAvatar->GetTotalMana();
			member->m_dwHealthCur = pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent;
			member->m_dwStaminaCur = pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent;
			member->m_dwManaCur = pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent;
		}

		return true;
	}

	return false;
}

/**
 *	Relays a member update to all members of the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member whose information has been updated.
 */
void cFellowship::RelayMemberUpdate(DWORD dwMemberGUID)
{
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		cClient* pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
		if (pcClient)
		{
			cMessage cmUpdate = UpdMemberMessage(itrMember->second.m_dwGUID, ++pcClient->m_dwF7B0Sequence, dwMemberGUID);
			pcClient->AddPacket( WORLD_SERVER, cmUpdate, 4 );
		}
	}
}

/**
 *	Relays a member deletion to all members of the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member who has left the fellowship.
 */
void cFellowship::RelayMemberDelete(DWORD dwMemberGUID)
{
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		cClient* pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
		if (pcClient)
		{
			cMessage cmRemove = RemMemberMessage(itrMember->second.m_dwGUID, ++pcClient->m_dwF7B0Sequence, dwMemberGUID);
			pcClient->AddPacket( WORLD_SERVER, cmRemove, 4 );
		}
	}
}

/**
 *	Relays a member dismissal to all members of the fellowship.
 *
 *	@param dwMemberGUID - The GUID for the member who has been dismissed from the fellowship.
 */
void cFellowship::RelayMemberDismiss(DWORD dwMemberGUID)
{
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		cClient* pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
		if (pcClient)
		{
			cMessage cmDismiss = DisMemberMessage(itrMember->second.m_dwGUID, ++pcClient->m_dwF7B0Sequence, dwMemberGUID);
			pcClient->AddPacket( WORLD_SERVER, cmDismiss, 4 );
		}
	}
}

/**
 *	Handles the message sent for joining the fellowship.
 *
 *	@parem dwClGUID - The client's GUID.
 *	@parem clF7B0Sequence - The client's present F7B0 sequence value.
 *
 *	@return cMessage - Returns an 0x0000F7B0 server message.
 */
cMessage cFellowship::JoinMessage(DWORD dwClGUID, DWORD dwClF7B0Sequence)
{
	cMessage cmMessage;

	cmMessage 	<< 0xF7B0L
				<< dwClGUID
				<< dwClF7B0Sequence
//				<< 0x02BEL;	//ToD
				<< 0x00AFL;

	cmMessage	<< WORD(this->m_Size)		// fellow count
				<< WORD(0x0010);			// unknown1

	// vector of fellowship member count (1 per member)
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		cmMessage	<< DWORD(itrMember->second.m_dwGUID)		// this member's GUID
					<< WORD(itrMember->second.m_wUnk1)			// unknown1
					<< WORD(itrMember->second.m_dwLevel)		// Level
					<< DWORD(itrMember->second.m_dwHealthTot)	// maximum Health
					<< DWORD(itrMember->second.m_dwStaminaTot)	// maximum Stamina
					<< DWORD(itrMember->second.m_dwManaTot)		// maximum Mana
					<< DWORD(itrMember->second.m_dwHealthCur)	// current Health
					<< DWORD(itrMember->second.m_dwStaminaCur)	// current Stamina
					<< DWORD(itrMember->second.m_dwManaCur)		// current Mana
					<< DWORD(itrMember->second.m_bShareLoot)	// share loot - 0x00 = no, 0x10 = yes (0 default)
					<< itrMember->second.m_szName.c_str();		// member's name
	}

	cmMessage	<< this->m_Name.c_str()			// fellowship name
				<< DWORD(this->m_LeaderGUID)	// leader's GUID
				<< 0x01L						// unknown3
				<< DWORD(this->m_ShareXP)		// share Experience - 0x00 = no, 0x10 = yes (1 default)
				<< DWORD(this->m_IsOpen)		// open fellowship - 0x00 = no, 0x10 = yes (0 default)
				<< 0x00L;						// unknown4
//				<< 0x00200000L					// unknown5
//				<< 0x00200000L;

	return cmMessage;
}

/**
 *	Handles the message sent for disbanding the fellowship.
 *
 *	@parem dwClGUID - The client's GUID.
 *	@parem clF7B0Sequence - The client's present F7B0 sequence value.
 *
 *	@return cMessage - Returns an 0x0000F7B0 server message.
 */
cMessage cFellowship::DisbandMessage(DWORD dwClGUID, DWORD dwClF7B0Sequence)
{
	cMessage cmMessage;

	cmMessage 	<< 0xF7B0L
				<< dwClGUID
				<< dwClF7B0Sequence
//				<< 0x0361L;	//ToD
				<< 0x00B3L;

	return cmMessage;
}

/**
 *	Handles the message sent for updating the fellowship member's information.
 *
 *	@parem dwClGUID - The client's GUID.
 *	@parem clF7B0Sequence - The client's present F7B0 sequence value.
 *	@parem clF7B0Sequence - The GUID for the member whose information has been updated.
 *
 *	@return cMessage - Returns an 0x0000F7B0 server message.
 */
cMessage cFellowship::UpdMemberMessage(DWORD dwClGUID, DWORD dwClF7B0Sequence, DWORD dwMemberGUID)
{
	UpdateMember(dwMemberGUID);

	cMessage cmMessage;

	cmMessage 	<< 0xF7B0L
				<< dwClGUID
				<< dwClF7B0Sequence
//				<< 0x02C0L;	//ToD
				<< 0x00B0L;

	// vector of fellowship member count (1 per member)
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		if (dwMemberGUID == itrMember->second.m_dwGUID)
		{
			cmMessage	<< DWORD(itrMember->second.m_dwGUID)		// this member's GUID
						<< WORD(itrMember->second.m_wUnk1)			// unknown1
						<< WORD(itrMember->second.m_dwLevel)		// Level
						<< DWORD(itrMember->second.m_dwHealthTot)	// maximum Health
						<< DWORD(itrMember->second.m_dwStaminaTot)	// maximum Stamina
						<< DWORD(itrMember->second.m_dwManaTot)		// maximum Mana
						<< DWORD(itrMember->second.m_dwHealthCur)	// current Health
						<< DWORD(itrMember->second.m_dwStaminaCur)	// current Stamina
						<< DWORD(itrMember->second.m_dwManaCur)		// current Mana
						<< DWORD(itrMember->second.m_bShareLoot)	// share loot - 0x00 = no, 0x10 = yes (0 default)
						<< itrMember->second.m_szName.c_str();		// member's name
		}
	}

	cmMessage	<< 0x01L;		// unknown; share Experience or Loot?

	return cmMessage;
}

/**
 *	Handles the message sent for a player who has left the fellowship.
 *
 *	@parem dwClGUID - The client's GUID.
 *	@parem clF7B0Sequence - The client's present F7B0 sequence value.
 *	@parem clF7B0Sequence - The GUID for the player who left the fellowship.
 *
 *	@return cMessage - Returns an 0x0000F7B0 server message.
 */
cMessage cFellowship::RemMemberMessage(DWORD dwClGUID, DWORD dwClF7B0Sequence, DWORD dwMemberGUID)
{
	cMessage cmMessage;

	cmMessage 	<< 0xF7B0L
				<< dwClGUID
				<< dwClF7B0Sequence
//				<< 0x00A3L;	//ToD
				<< 0x00A7L;

	cmMessage	<< dwMemberGUID;

	return cmMessage;
}

/**
 *	Handles the message sent for a player who has been dismissed from the fellowship.
 *
 *	@parem dwClGUID - The client's GUID.
 *	@parem clF7B0Sequence - The client's present F7B0 sequence value.
 *	@parem clF7B0Sequence - The GUID for the player who been dismissed from the fellowship.
 *
 *	@return cMessage - Returns an 0x0000F7B0 server message.
 */
cMessage cFellowship::DisMemberMessage(DWORD dwClGUID, DWORD dwClF7B0Sequence, DWORD dwMemberGUID)
{
	cMessage cmMessage;

	cmMessage 	<< 0xF7B0L
				<< dwClGUID
				<< dwClF7B0Sequence
//				<< 0x00A4L;	//ToD
				<< 0x00B1L;

	cmMessage	<< dwMemberGUID;

	return cmMessage;
}

/**
 *	Handles the distribution of experience among fellowship members.
 *
 *	This function only accounts for experience earned by fellowship members by hunting creatures.
 *
 *	@parem dwMemberGUID - The GUID of the member who has earned the experience.
 *	@parem memberLoc - The location of the member who has earned the experience.
 *	@parem dwExperience - The amount of experience earned.
 */
void cFellowship::DistributeXP(DWORD dwMemberGUID, cLocation memberLoc, DWORD dwExperience)
{
	if (this->m_ShareXP)
	{
		if (this->m_ProportionXP)
		{
			std::map<DWORD, FellowMem>::iterator itrMember;
			for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
			{
				cClient* pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
				if (pcClient)
				{
					cAvatar* pcAvatar = pcClient->m_pcAvatar;
					if (pcAvatar)
					{
						dwExperience = dwExperience * itrMember->second.m_fShareOfXP;
						pcAvatar->UpdateFellowshipExp(dwExperience);
					}
				}
			}
		}
		else
		{
			dwExperience = dwExperience * CalcFellowFactor(this->GetSize());

			std::map<DWORD, FellowMem>::iterator itrMember;
			for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
			{
				cClient* pcClient = cClient::FindClient(itrMember->second.m_dwGUID);
				if (pcClient)
				{
					cAvatar* pcAvatar = pcClient->m_pcAvatar;
					if (pcAvatar)
					{
						dwExperience = dwExperience * CalcDistanceFactor(memberLoc, pcAvatar->m_Location);
						pcAvatar->UpdateFellowshipExp(dwExperience);
					}
				}
			}
		}
	}
	else
	{
		cClient* pcClient = cClient::FindClient(dwMemberGUID);
		if (pcClient)
		{
			cAvatar* pcAvatar = pcClient->m_pcAvatar;
			if (pcAvatar)
			{
				pcAvatar->UpdateFellowshipExp(dwExperience);
			}
		}
	}
}

/**
 *	Calculates the proportion by which to distribute experience among fellowship members.
 */
void cFellowship::CalcShareXP()
{
	int leaderLevel, levelDiff = 0;
	int minLevel = MAX_LEVEL;

	FellowMem* leader = this->FindMember(this->m_LeaderGUID);
	leaderLevel = leader->m_dwLevel;

	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		if (abs(leaderLevel - itrMember->second.m_dwLevel) > levelDiff)
			levelDiff = abs(leaderLevel - itrMember->second.m_dwLevel);
		if (itrMember->second.m_dwLevel < minLevel)
			minLevel = itrMember->second.m_dwLevel;
	}

	if (levelDiff > 10 && minLevel < 50)
	{
		this->m_ProportionXP = true;
		CalcProportionXP(this->m_Size, levelDiff, minLevel);
	}
	else
	{
		this->m_ProportionXP = false;
		CalcNonProportionXP(this->m_Size);
	}
}

/**
 *	Calculates the distribution of proportional experience among fellowship members.
 *
 *	@parem numFellows - The number of fellows in the fellowship.
 *	@parem levelDiff - The difference between the lowest and highest level members of the fellowship.
 *	@parem minLevel - The lowest level of all members of the fellowship.
 */
void cFellowship::CalcProportionXP(int numFellows, int levelDiff, int minLevel)
{
	std::map<DWORD, FellowMem>::iterator itrMember;
	for ( itrMember = this->members.begin() ; itrMember != this->members.end() ; ++itrMember )
	{
		// TODO: Find true formula.
		itrMember->second.m_fShareOfXP = (100/numFellows) + (itrMember->second.m_dwLevel - minLevel);
	}
}

/**
 *	Calculates the distribution of non-proportional experience among fellowship members.
 *
 *	@parem numFellows - The number of fellows in the fellowship.
 */
void cFellowship::CalcNonProportionXP(int numFellows)
{
}

/**
 *	Calculates the fellow factor for a fellowship.
 *
 *	@parem numFellows - The number of fellows in the fellowship.
 *
 *	@return float - Returns the fellowship factor.
 */
float cFellowship::CalcFellowFactor(int numFellows)
{
/*
	Pre-ToD?:
* 2  players in fellowship earn 75% of normal experience
* 3  players in fellowship earn 60% of normal experience
* 4  players in fellowship earn 55% of normal experience
* 5  players in fellowship earn 50% of normal experience
* 6  players in fellowship earn 45% of normal experience
* 7  players in fellowship earn 40% of normal experience
* 8  players in fellowship earn 35% of normal experience
* 9  players in fellowship earn 31% of normal experience
* 10 players in fellowship earn 28% of normal experience

	ToD:
* 2  players in fellowship earn 75% of normal experience
* 3  players in fellowship earn 60% of normal experience
* 4  players in fellowship earn 55% of normal experience
* 5  players in fellowship earn 50% of normal experience
* 6  players in fellowship earn 45% of normal experience
* 7  players in fellowship earn 40% of normal experience
* 8  players in fellowship earn 35% of normal experience
* 9  players in fellowship earn 30% of normal experience

*/
	float fellowFactor;

	if (OLD_PASSTHROUGH)
	{
		switch (numFellows)
		{
			case 1:		fellowFactor = 1.00f;	break;
			case 2:		fellowFactor = 0.75f;	break;
			case 3:		fellowFactor = 0.60f;	break;
			case 4:		fellowFactor = 0.55f;	break;
			case 5:		fellowFactor = 0.50f;	break;
			case 6:		fellowFactor = 0.45f;	break;
			case 7:		fellowFactor = 0.40f;	break;
			case 8:		fellowFactor = 0.35f;	break;
			case 9:		fellowFactor = 0.31f;	break;
			case 10:	fellowFactor = 0.28f;	break;
			default:	fellowFactor = 1.00f;	break;
		}
	}
	else
	{
		switch (numFellows)
		{
			case 1:		fellowFactor = 1.00f;	break;
			case 2:		fellowFactor = 0.75f;	break;
			case 3:		fellowFactor = 0.60f;	break;
			case 4:		fellowFactor = 0.55f;	break;
			case 5:		fellowFactor = 0.50f;	break;
			case 6:		fellowFactor = 0.45f;	break;
			case 7:		fellowFactor = 0.40f;	break;
			case 8:		fellowFactor = 0.35f;	break;
			case 9:		fellowFactor = 0.30f;	break;
			default:	fellowFactor = 1.00f;	break;
		}
	}

	return fellowFactor * SHARED_XP_MULT;
}

/**
 *	Calculates the distance factor for a member of a fellowship.
 *
 *	This function is used to determine how a member's experience earnings are affected 
 *	by his/her distance from the fellow that earned the experience.
 *
 *	@parem earnMemLoc - The location of the fellow that earned the experience.
 *	@parem recvMemLoc - The location of the fellow receiving apportioned experience.
 *
 *	@return float - Returns the distance factor.
 */
float cFellowship::CalcDistanceFactor(cLocation earnMemLoc, cLocation recvMemLoc)
{

//	OLD:  < ~8 = 100%; 48 ~= 50%
//	NEW:  < ~2.5 = 100%; 5 ~= 0%

	float flDistance = cPhysics::GetRange(earnMemLoc, recvMemLoc);

	if (OLD_FELLOW_RANGE)
	{
		if (flDistance < 8 * FELLOW_RANGE_MULT)
		{
			return 1;
		}
		else
		{
			return 1;	//TODO: formula
		}
	}
	else
	{
		if (flDistance < 2.5 * FELLOW_RANGE_MULT)
		{
			return 1;
		}
		else
		{
			return 1;	//TODO: formula
		}
	}
}