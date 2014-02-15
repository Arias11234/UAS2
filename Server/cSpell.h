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
 *	@file cSpell.h
 */

#if !defined(AFX_CSPELL_H__08D0B2E9_9DDB_4F52_B2A8_8847149A4EAB__INCLUDED_)
#define AFX_CSPELL_H__08D0B2E9_9DDB_4F52_B2A8_8847149A4EAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <winsock2.h>
#include "Shared.h"
#include "Message.h"
#include "Client.h"
#include "WorldManager.h"
#include "MasterServer.h"

class cSpell
{
	friend class cMasterServer;
public:
	cSpell( DWORD dwSpellID, BOOL fAddToHash = TRUE )
		:	m_dwSpellID( dwSpellID ),
			m_pcPrev		( NULL ),
			m_pcNext		( NULL )
	{
		m_dwIconID		=		0x0L;
		m_dwEffect		=		0x0L;
		m_fResearchable =		false;
		m_dwManaCost	=		0x0L;
		m_flUnkFloat1	=		0.0f;
		m_flUnkFloat2	=		0.0f;
		m_dwDifficulty	=		0x0L;
		m_fEconomy		=		false;
		m_flSpeed		=		0;
		m_dwType		=		0x0L;
		m_iDuration		=		0;
		for (int i = 0; i < 9; i++)
			m_dwComponents[i] = 0x0L;
		m_dwEffectAnim	=		0x0L;
		m_dwLevel		=		0x0L;

		if( fAddToHash )
		Hash_Add( this );
	}

	virtual ~cSpell()
	{
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[m_dwSpellID] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}

	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline cSpell *Hash_New( DWORD& dwSpellID )
	{
		cSpell *pcSpell = Hash_Find( dwSpellID );
		if ( pcSpell )
			return pcSpell;

		return new cSpell( dwSpellID ) ;
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cSpell *pcSpell );
	static cSpell	*FindSpell	( DWORD dwSpellID );	

	BYTE				GetWindup	( );
	int					GetWindupDelay ( );
	BYTE				GetCastAnim ( );
	int					GetCastAnimDelay ( );
	cMessage			GetCastWords( );

	DWORD				m_dwSpellID;
	std::string			m_strName;
	std::string			m_strDesc;
	std::string			m_strSchool;
	DWORD				m_dwIconID;
	DWORD				m_dwEffect;
	BOOL				m_fResearchable;
	DWORD				m_dwManaCost;
	float				m_flUnkFloat1;
	float				m_flUnkFloat2; //Target related, decreases with spell lvl
	DWORD				m_dwDifficulty; // Skill level required
	BOOL				m_fEconomy; // does ManaC need to be checked
	float				m_flSpeed; // Guessing windup speed, increases with spell lvl
	DWORD				m_dwType;
	int					m_iDuration;
	DWORD				m_dwComponents[9];
	DWORD				m_dwEffectAnim;
	DWORD				m_dwLevel;

	cSpell				*m_pcNext;
	cSpell				*m_pcPrev;

private:
	static cSpell *m_lpcHashTable[3000];

	static inline Hash_Add( cSpell *pcSpell )
	{
		const DWORD dwSpellID = pcSpell->m_dwSpellID;
		if ( !m_lpcHashTable[dwSpellID] )
			m_lpcHashTable[dwSpellID] = pcSpell;
		else
		{
			pcSpell->m_pcNext	=	m_lpcHashTable[dwSpellID];
			m_lpcHashTable[dwSpellID]->m_pcPrev	= pcSpell;
			m_lpcHashTable[dwSpellID]			= pcSpell;
		}
	}

	
	static inline cSpell *Hash_Find( DWORD& dwSpellID )
	{
		const DWORD dwSpell = dwSpellID;
		cSpell *pcSpell = m_lpcHashTable[dwSpell];
		while ( pcSpell )
		{
			if ( pcSpell->CompareSpell( dwSpellID ) )	return pcSpell;
			else	
				pcSpell = pcSpell->m_pcNext;
		}
		return NULL;
	}

		
	inline BOOL CompareSpell( DWORD& dwSpellID )
	{
		if ( dwSpellID == m_dwSpellID )
			return true;
		else
			return false;
	}
};

class cSpellComp
{
	friend class cMasterServer;
	friend class cSpell;
public:
	cSpellComp( DWORD dwCompID, BOOL fAddToHash = TRUE )
		:	m_dwCompID( dwCompID ),
			m_pcPrev		( NULL ),
			m_pcNext		( NULL )
	{
		m_dwIconID		=		0x0L;
		m_flChargeID	=		0.0f;
		m_flBurnRate	=		0.0f;
		m_dwAnimID		=		0x0L;

		if( fAddToHash )
		Hash_Add( this );
	}

	virtual ~cSpellComp()
	{
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[m_dwCompID] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}

	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline cSpellComp *Hash_New( DWORD& dwCompID )
	{
		cSpellComp *pcComp = Hash_Find( dwCompID );
		if ( pcComp )
			return pcComp;

		return new cSpellComp( dwCompID ) ;
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cSpellComp *pcComp );
	static cSpellComp	*FindComp	( DWORD dwCompID );

	DWORD				m_dwCompID;
	std::string			m_strName;
	std::string			m_strType;
	std::string			m_strWords;
	DWORD				m_dwIconID;
	float				m_flChargeID;
	float				m_flBurnRate;
	DWORD				m_dwAnimID;

	cSpellComp			*m_pcNext;
	cSpellComp			*m_pcPrev;

private:
	static cSpellComp *m_lpcHashTable[200];

	static inline Hash_Add( cSpellComp *pcComp )
	{
		const DWORD dwCompID = pcComp->m_dwCompID;
		if ( !m_lpcHashTable[dwCompID] )
			m_lpcHashTable[dwCompID] = pcComp;
		else
		{
			pcComp->m_pcNext	=	m_lpcHashTable[dwCompID];
			m_lpcHashTable[dwCompID]->m_pcPrev	= pcComp;
			m_lpcHashTable[dwCompID]			= pcComp;
		}
	}

	
	static inline cSpellComp *Hash_Find( DWORD& dwCompID )
	{
		const DWORD dwComp = dwCompID;
		cSpellComp *pcComp = m_lpcHashTable[dwComp];
		while ( pcComp )
		{
			if ( pcComp->CompareComp( dwCompID ) )	return pcComp;
			else	
				pcComp = pcComp->m_pcNext;
		}
		return NULL;
	}

		
	inline BOOL CompareComp( DWORD& dwCompID )
	{
		if ( dwCompID == m_dwCompID )
			return true;
		else
			return false;
	}
};

class cEnchantment
{

public:
	cEnchantment( DWORD dwSpellID, WORD wFamily, DWORD dwDifficulty, double dDuration, DWORD dwCasterGUID, DWORD dwTargetGUID, double dCastTime, DWORD dwFlags, DWORD dwKey, float flValue )
		:		m_dwSpellID			( dwSpellID ),
				m_wFamily			( wFamily ),
				m_dwDifficulty		( dwDifficulty ),
				m_dDuration			( dDuration ),
				m_dwCasterGUID		( dwCasterGUID ),
				m_dwTargetGUID		( dwTargetGUID ),
				m_dCastTime			( dCastTime ),
				m_dwFlags			( dwFlags ),
				m_dwKey				( dwKey ),
				m_flValue			( flValue )
	{
		m_wLayer		= 0x0001;
		m_wUnknown0		= 0x0000;
		m_dwUnknown1	= 0x0L;
		m_dwUnknown2	= 0x0L;
		m_dwUnknown3	= 0x0L;

		cObject *pcObject = cWorldManager::FindObject( m_dwTargetGUID );

		if (pcObject)
		{
			for ( iterEnchantment_lst itEnchantment = pcObject->m_lstEnchantments.begin( ); itEnchantment != pcObject->m_lstEnchantments.end( ); ++itEnchantment )
			{
				if ( (*itEnchantment)->m_dwSpellID == m_dwSpellID && (*itEnchantment)->m_wLayer >= m_wLayer )
					m_wLayer = (*itEnchantment)->m_wLayer + 1;
			}

			pcObject->m_lstEnchantments.push_back( this );
			cMasterServer::m_lstEnchantments.push_back( this );
		}
		else
		{
			cClient *pcClient = cClient::FindClient( m_dwTargetGUID );

			if (pcClient)
			{
				for ( iterEnchantment_lst itEnchantment = pcClient->m_pcAvatar->m_lstEnchantments.begin( ); itEnchantment != pcClient->m_pcAvatar->m_lstEnchantments.end( ); ++itEnchantment )
				{
					if ( (*itEnchantment)->m_dwSpellID == m_dwSpellID && (*itEnchantment)->m_wLayer >= m_wLayer )
						m_wLayer = (*itEnchantment)->m_wLayer + 1;
				}

				pcClient->m_pcAvatar->m_lstEnchantments.push_back( this );
				cMasterServer::m_lstEnchantments.push_back( this );
				pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->AddEnchant( ++pcClient->m_dwF7B0Sequence, m_dwSpellID, m_wLayer, m_wFamily, m_dwDifficulty, m_dDuration, m_dwCasterGUID, m_dCastTime, m_dwFlags, m_dwKey, m_flValue ), 4);	
			}
		}
	}

	~cEnchantment()
	{
		cObject *pcObject = cWorldManager::FindObject( m_dwTargetGUID );

		if (pcObject)
		{
			for ( iterEnchantment_lst itEnchantment = pcObject->m_lstEnchantments.begin(); itEnchantment != pcObject->m_lstEnchantments.end( ); ++itEnchantment )
			{
				if ( (*itEnchantment)->m_dwSpellID == m_dwSpellID && (*itEnchantment)->m_wLayer == m_wLayer )
				{
					pcObject->m_lstEnchantments.erase( itEnchantment );
					itEnchantment = pcObject->m_lstEnchantments.end();
				}
			}
		}
		else
		{
			cClient *pcClient = cClient::FindClient( m_dwTargetGUID );

			if (pcClient)
			{
				for ( iterEnchantment_lst itEnchantment = pcClient->m_pcAvatar->m_lstEnchantments.begin(); itEnchantment != pcClient->m_pcAvatar->m_lstEnchantments.end( ); ++itEnchantment )
				{
					if ( (*itEnchantment)->m_dwSpellID == m_dwSpellID && (*itEnchantment)->m_wLayer >= m_wLayer )
					{
						pcClient->m_pcAvatar->m_lstEnchantments.erase( itEnchantment );
						itEnchantment = pcClient->m_pcAvatar->m_lstEnchantments.end();
					}
				}
				pcClient->AddPacket( WORLD_SERVER, pcClient->m_pcAvatar->RemoveEnchant( ++pcClient->m_dwF7B0Sequence, m_dwSpellID, m_wLayer ), 4);	
			}
		}
	}

	DWORD		m_dwSpellID;
	WORD		m_wLayer;
	WORD		m_wFamily;
	WORD		m_wUnknown0;
	DWORD		m_dwDifficulty;
	double		m_dDuration;
	DWORD		m_dwCasterGUID;
	DWORD		m_dwTargetGUID;
	DWORD		m_dwUnknown1;
	DWORD		m_dwUnknown2;
	double		m_dCastTime;
	DWORD		m_dwFlags;
	DWORD		m_dwKey;
	float		m_flValue;
	DWORD		m_dwUnknown3;
};

#endif // !defined(AFX_CSPELL_H__08D0B2E9_9DDB_4F52_B2A8_8847149A4EAB__INCLUDED_)