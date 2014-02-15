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
 *	@file Fellowship.h
 */

#ifndef __FELLOWSHIPS_H
#define __FELLOWSHIPS_H

#pragma warning(disable:4786)	//warning: identifier was truncated to '255' characters in the browser information
#include <map>

#include "Avatar.h"
#include "Shared.h"

struct FellowMem
{
    DWORD		m_dwGUID;
	WORD		m_wUnk1;
	DWORD		m_dwLevel;		// Level
	DWORD		m_dwHealthTot;	// maximum Health
	DWORD		m_dwStaminaTot;	// maximum Stamina
	DWORD		m_dwManaTot;	// maximum Mana
	DWORD		m_dwHealthCur;	// current Health
	DWORD		m_dwStaminaCur;	// current Stamina
	DWORD		m_dwManaCur;	// current Mana
	bool		m_bShareLoot;	// share loot - 0x00 = no, 0x10 = yes
	std::string	m_szName;
	
	float		m_fShareOfXP;
	ULONG		m_timeJoin;
};

class cFellowship
{
	friend class cAvatar;
	friend class cMasterServer;

    public:
        cFellowship();
        ~cFellowship();

		static DWORD	NewFellowship	( DWORD dwLeaderGUID, std::string name, char felName[50], bool shareXP, bool shareLoot );
		
        DWORD		CreateFellowship	( DWORD dwLeaderGUID, std::string name, char felName[50], bool shareXP, bool shareLoot );
        void		Disband				( );
		void		ClearMembers		( );
		
		static		cFellowship*	GetFellowshipByID	( DWORD dwFellowshipID );
		DWORD		GetID()				{ return m_ID; }
        DWORD		GetLeader()			{ return m_LeaderGUID; }
		DWORD		GetSize()			{ return m_Size; }
		std::string GetName()			{ return m_Name.c_str(); }
        ULONG		GetCreationTime()	{ return m_CreationTime; }
		bool		GetIsOpen()			{ return m_IsOpen; }
        
		typedef		std::map<DWORD, FellowMem> MemberList;

		FellowMem*	FindMember	( DWORD dwMemberGUID ) { return &this->members.find(dwMemberGUID)->second; }

		bool		SetLeader			( DWORD dwNewLeaderGUID );
		bool		SetOpenClose		( DWORD dwIsOpen );
		bool		AddMember			( DWORD dwAvatarGUID );
		bool		RemMember			( DWORD dwMemberGUID );
		bool		DismissMember		( DWORD dwMemberGUID );
		bool		MemberDeath			( DWORD dwMemberGUID );

		bool		InsertMember		( DWORD dwMemberGUID );
		bool		DeleteMember		( DWORD dwMemberGUID, bool wasDismissed=false );
		bool		UpdateMember		( DWORD dwMemberGUID );

		void		RelayMemberUpdate	( DWORD dwMemberGUID );
		void		RelayMemberDelete	( DWORD dwMemberGUID );
		void		RelayMemberDismiss	( DWORD dwMemberGUID );

		cMessage	JoinMessage			( DWORD dwClGUID, DWORD dwClF7B0Sequence );
		cMessage	DisbandMessage		( DWORD dwClGUID, DWORD dwClF7B0Sequence );
		cMessage	UpdMemberMessage	( DWORD dwClGUID, DWORD dwClF7B0Sequence, DWORD dwMemberGUID );
		cMessage	RemMemberMessage	( DWORD dwClGUID, DWORD dwClF7B0Sequence, DWORD dwMemberGUID );
		cMessage	DisMemberMessage	( DWORD dwClGUID, DWORD dwClF7B0Sequence, DWORD dwMemberGUID );
		
		void		DistributeXP		( DWORD dwMemberGUID, cLocation memberLoc, DWORD dwExperience );
		void		CalcShareXP			( );
		void		CalcProportionXP	( int numFellows, int levelDiff, int minLevel );
		void		CalcNonProportionXP	( int numFellows );
		float		CalcFellowFactor	( int numFellows );
		float		CalcDistanceFactor	( cLocation earnMemLoc, cLocation recvMemLoc );

		MemberList	members;
	
	protected:
        DWORD		m_ID;
        std::string	m_Name;
        DWORD		m_LeaderGUID;
		int			m_Size;
		ULONG		m_CreationTime;
		bool		m_IsOpen;			// opened/closed fellowship (allows all members to recruit) (0 = no; 1 = yes)
		bool		m_ShareLoot;		// share loot among fellows (0 = no; 1 = yes)
		bool		m_ShareXP;			// share experience among fellows (0 = no; 1 = yes)
		bool		m_ProportionXP;		// proportion shared experience (according to level) (0 = no; 1 = yes)
};

#endif	// #ifndef __FELLOWSHIPS_H