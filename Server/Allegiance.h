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
 *	@file Allegiance.h
 */

#ifndef __ALLEGIANCES_H
#define __ALLEGIANCES_H

#pragma warning(disable:4786)	//warning: identifier was truncated to '255' characters in the browser information
#include <map>
//#include <vector>

#include "Avatar.h"
#include "Shared.h"

struct Member
{
	// allegiance table
    DWORD		m_dwGUID;
    std::string	m_szName;
	bool		m_online;
    BYTE		m_bGender;
	BYTE		m_bRace;
	BYTE		m_bRank;
	WORD		m_wLeadership;
	WORD		m_wLoyalty;
	DWORD		m_dwPatron;
	DWORD		m_dwVassals[MAX_VASSALS];	// vassals determined at run-time
	DWORD		m_dwFollowers;	// number of personal followers
	float		m_avgIGSworn;	// average in-game sworn time of vassals
	float		m_avgRTSworn;	// average real-time sworn time of vassals
	DWORD		m_passupXP;		// experience generated
	DWORD		m_pendingXP;	// experience pending since previous login

	// avatar table
	DWORD		m_dwLevel;
	ULONG		m_timeBirth;
	ULONG		m_timeRLSwear;	// sworn time, real-life
	ULONG		m_timeIGTotal;	// total time in-game (character age)
	ULONG		m_timeIGSwear;	// sworn time, in-game (character age)
};

class cAllegiance
{
	friend class cMasterServer;

    public:
        cAllegiance();
        ~cAllegiance();

		void		LoadAllegiance		(DWORD ID, DWORD leaderGUID, char name[40], char MOTD[255]);
        void		CreateAllegiance	( DWORD leaderGUID );
        void		Disband				( );

		typedef		std::map<DWORD, Member> MemberList;

		Member*		FindMember	( DWORD memberGUID ) { return &this->members.find(memberGUID)->second; }

		static cAllegiance*	GetAllegianceByID		( DWORD AllegianceID );
        DWORD		GetID()				{ return m_ID; }
        DWORD		GetLeader()			{ return m_LeaderGUID; }
		DWORD		GetSize()			{ return m_Size; }
		std::string GetName()			{ return m_Name.c_str(); }
        std::string GetMOTD()			{ return m_MOTD.c_str(); }

        static void		SetLeader					( DWORD allegianceID, DWORD leaderGUID );
		static bool		AddNewMember				( DWORD dwAvatarGUID, DWORD dwPatronGUID, DWORD oldAllegianceID, DWORD newAllegianceID );
		static bool		AddRemMember				( Member* member, DWORD oldAllegianceID, DWORD newAllegianceID );
		static bool		RemMember					( DWORD dwRemMemberGUID, DWORD oldAllegianceID, bool isDisbanding=false );
		static bool		SwitchMonarchAndVassals		( Member* member, DWORD oldAllegianceID, DWORD newAllegianceID );
		static bool		SwitchNonMonarchVassals		( Member* member, DWORD oldAllegianceID, DWORD newAllegianceID );

		void		InsertMember		( Member member, DWORD dwPatronGUID );
		void		EraseMember			( Member* member );
		void		EraseMember			( DWORD dwMemberGUID );

		bool		AddDirectVassal		( Member* memPatron, Member* memVassal );
		bool		AddDirectVassal		( DWORD dwPatronGUID, DWORD dwVassalGUID );
		bool		RemDirectVassal		( Member* memVassal );
		bool		RemDirectVassal		( DWORD dwVassalGUID );

		void		SetMOTD				( std::string motd );

		std::string GetMemberName		( Member* member )		{ return member->m_szName.c_str(); }
		std::string GetMemberName		( DWORD dwMemberGUID )	{ Member* member = FindMember(dwMemberGUID); return member->m_szName.c_str(); }

		int			GetVassalCount		( Member* member );
		int			GetVassalCount		( DWORD dwMemberGUID );
		int			GetFollowerCount	( Member* member );
		int			GetFollowerCount	( DWORD dwMemberGUID );
		
		static void	UpdatePlayerAllegID			( Member* member, DWORD newAllegianceID );
		static void	UpdatePlayerAllegID			( DWORD dwPlayerGUID, DWORD newAllegianceID );
		void		UpdateFollowers				( Member* member, int numFollowers );
		void		UpdateFollowers				( DWORD dwMemberGUID, int numFollowers );
		void		UpdateRank					( Member* member );
		void		UpdateRank					( DWORD dwMemberGUID );

		void		VassalPassupXP						( Member* memVassal, UINT64 experience, bool isGrandVassal=false );
		void		UpdateGeneratedXP					( Member* member, UINT64 experience );
		void		UpdateEarnedXP						( Member* member, UINT64 experience );
		UINT64		CalcVassalGeneratedXP				( Member* memVassal, UINT64 experience, bool isGrandPatron=false );
		UINT64		CalcPatronReceivedXP				( Member* memPatron, UINT64 experience, bool isGrandPatron=false );
		void		UpdateAverageSwornTimeOfVassals		( Member* memPatron );
		float		CalcVassalFactor					( int numVassals );

		cMessage	GetAllegianceInfo	( DWORD avatarGUID );

		static void	LoadMembersFromDB	( cAllegiance* aAllegiance );

		static void		UpdateAvatarAllegIDDB		( DWORD dwGUID, DWORD allegianceID );
		static void		UpdateMemberAllegIDDB		( DWORD dwGUID, DWORD allegianceID );
		static void		UpdateAvatarRecordDB		( DWORD dwAvatarGUID );
		static void		AddMemberRecordDB			( DWORD allegianceID, DWORD dwAvatarGUID, DWORD dwPatronGUID );
		static void		DeleteMemberRecordDB		( DWORD allegianceID, DWORD dwMemberGUID );
		static void		UpdateMemberRecordDB		( Member* member );
		static void		UpdateMemberXPPassupDB		( DWORD memberGUID, UINT64 passedXP );
		static void		UpdateMemberXPPendingDB		( DWORD memberGUID, UINT64 pendingXP );
		static void		UpdateMemberFollowersDB		( DWORD memberGUID, int numFollowers );
		static void		UpdateMemberRankDB			( DWORD memberGUID, int rank );
		static void		UpdateMemberPatronDB		( DWORD memberGUID, DWORD patronGUID );

		MemberList	members;

    protected:
        DWORD		m_ID;
        std::string	m_Name;
        DWORD		m_LeaderGUID;
        std::string	m_MOTD;
		int			m_Size;			// calculated at run-time
		ULONG		m_CreationTime;
};

#endif	// #ifndef __ALLEGIANCES_H