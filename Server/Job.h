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
 *	@file Job.h
 */

#ifndef __JOB_H
#define __JOB_H

#include <winsock2.h>
#include <list>

#include "Shared.h"

#define		JOB_NORMAL		0
#define		JOB_DISABLE		1
#define		JOB_REMOVE		2

#define		JOB_INFINITE	-1

typedef int (*MessageCall)( LPVOID a, LPVOID b );

class cJob
{
public:
	cJob( )
		:	m_fEnabled( FALSE )
	{
	}
	cJob( MessageCall func, LPVOID wp, LPVOID lp, int iFreq, int iNumReps, char *szName, int iJobID )
		:	m_mcFunction( func ),
			m_wParam	( wp ),
			m_lParam	( lp ),
			m_iNumTimes	( iNumReps ),
			m_iJobID	( iJobID ),
			m_iCounter	( 0 ),
			m_iFrequency( iFreq ),
			m_fEnabled	( TRUE )
	{
		lstrcpy( m_szJobName, szName );
	}

	~cJob( )
	{
	}

	int				Tick		( );

	inline LPVOID	GetWParam	( )				{ return m_wParam;		}
	inline LPVOID	GetLParam	( )				{ return m_lParam;		}
	inline void		SetWParam	( LPVOID wp )	{ m_wParam = wp;		}
	inline void		SetLParam	( LPVOID lp )	{ m_lParam = lp;		}
	inline int		GetJobID	( )				{ return m_iJobID;		}
	inline char		*GetName	( )				{ return m_szJobName;	}
	inline void		SetFrequency( int iFreq )	{ m_iFrequency = iFreq;	}
	inline int		GetFrequency( )				{ return m_iFrequency;	}
	inline void		Enable		( BOOL fState )	{ m_fEnabled = fState;	}

private:
	MessageCall m_mcFunction;
	LPVOID		m_wParam;
	LPVOID		m_lParam;
	int			m_iJobID;
	int			m_iFrequency;
	int			m_iNumTimes;
	int			m_iCounter;
	char		m_szJobName[100];
	BOOL		m_fEnabled;
};


class cJobPool
{
public:
	cJobPool( )
		:	m_iJobID( 0 )
	{
		NextTickTime = clock()/(CLOCKS_PER_SEC/TICKS_PER_SEC);
	}

	~cJobPool( )
	{
		while ( !m_lstJobList.empty( ) )
		{
			SAFEDELETE( m_lstJobList.front( ) )
			m_lstJobList.pop_front( );
		}
	}

	void	Tick			( );
	int		CreateJob		( MessageCall mc, LPVOID wParam, LPVOID lParam, char *szName, int iFreq, int iNumTimes );
	cJob	*GetJobByID		( int iID );
	int		GetJobIDByName	( char *szName );
	void	RemoveJob		( int iID );

	static clock_t	longtime;
	static clock_t	NextTickTime;

private:
	int					m_iJobID;
	std::list< cJob * > m_lstJobList;
};

class cWarJobParam
{
	friend class cAvatar;
public:
	cWarJobParam()
	{
	}

	cWarJobParam( DWORD SpellID, DWORD CasterGUID, DWORD TargetGUID, cLocation CastLocation, DWORD SpellSequence)
		:	m_dwSpellID			( SpellID ),
			m_dwCasterGUID		( CasterGUID ),
			m_dwTargetGUID		( TargetGUID ),
			m_CastLocation		( CastLocation ),
			m_dwSpellSequence	( SpellSequence )
	{
	}

	~cWarJobParam()
	{
	}

	inline DWORD		GetSpellID			( )		{ return m_dwSpellID;		}
	inline DWORD		GetCasterGUID		( )		{ return m_dwCasterGUID;	}
	inline DWORD		GetTargetGUID		( )		{ return m_dwTargetGUID;	}
	inline cLocation	GetCastLocation		( )		{ return m_CastLocation;	}
	inline DWORD		GetSpellSequence	( )		{ return m_dwSpellSequence;	}

private:
	DWORD		m_dwSpellID;
	DWORD		m_dwCasterGUID;
	DWORD		m_dwTargetGUID;
	cLocation   m_CastLocation;
	DWORD		m_dwSpellSequence;

};

class cSpellMoveParam
{
public:
	cSpellMoveParam()
	{
	}

	cSpellMoveParam( DWORD CasterGUID, cLocation CastLocation, DWORD WarSpellGUID )
		:	m_dwCasterGUID		( CasterGUID ),
			m_CastLocation		( CastLocation ),
			m_dwWarSpellGUID	( WarSpellGUID )
	{
	}

	~cSpellMoveParam()
	{
	}

	inline DWORD		GetCasterGUID		( )		{ return m_dwCasterGUID;	}
	inline cLocation	GetCastLocation		( )		{ return m_CastLocation;	}
	inline DWORD		GetWarSpellGUID		( )		{ return m_dwWarSpellGUID;	}

private:
	DWORD		m_dwCasterGUID;
	cLocation	m_CastLocation;
	DWORD		m_dwWarSpellGUID;

};

class cMonsterDeathParam
{
public:
	cMonsterDeathParam()
	{
	}

	cMonsterDeathParam( DWORD MonsterGUID, DWORD MonsterModelID, DWORD ClientGUID )
		:	m_dwMonsterGUID		( MonsterGUID ),
			m_dwMonsterModelID	( MonsterModelID ),
			m_dwClientGUID		( ClientGUID  )
	{
	}

	~cMonsterDeathParam()
	{
	}

	inline DWORD		GetMonsterGUID		( )		{ return m_dwMonsterGUID;	}
	inline DWORD		GetMonsterModelID	( )		{ return m_dwMonsterModelID;}
	inline DWORD		GetClientGUID		( )		{ return m_dwClientGUID;	}

private:
	DWORD		m_dwMonsterGUID;
	DWORD		m_dwMonsterModelID;
	DWORD		m_dwClientGUID;

};


class cAvatarGenericAnimation
{
	public:
	cAvatarGenericAnimation()
	{
	}
	
	~cAvatarGenericAnimation()
	{
	}
};

#endif	// #ifndef __JOB_H