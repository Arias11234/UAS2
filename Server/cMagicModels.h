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

#if !defined(AFX_CMAGICMODELS_H__F2C9124C_F6F0_4FCD_B4FC_1C85BE6113E1__INCLUDED_)
#define AFX_CMAGICMODELS_H__F2C9124C_F6F0_4FCD_B4FC_1C85BE6113E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <winsock2.h>
#include "shared.h"
#define MAGIC_MODEL_TABLE_SIZE 1000

class cMagicModels  
{
	friend class cMasterServer;

public:
	cMagicModels( DWORD dwSpellID, BOOL fAddToHash = TRUE )
			:	m_dwSpellID( dwSpellID ),
				m_pcPrev		( NULL ),
				m_pcNext		( NULL )
	{
		m_dwModelNumber		=	0x0L;
		m_dwFlags1			=	0x0000FB85; //varies with spell
		m_wParticle			=	0x0000;
		m_wSoundSet			=	0x0000; // varies with spell
		m_flScale			=	1.0f; // varies by spell level
		m_wModel			=	0x0000;
		m_wIcon				=	0x0000;
		m_wAssociatedSpell	=	0x0000;

		if( fAddToHash )
		Hash_Add( this );
	}

	virtual ~cMagicModels()
	{
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[m_dwSpellID] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}
		
	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline cMagicModels *Hash_New( DWORD& dwSpellID )
	{
		cMagicModels *pcModel = Hash_Find( dwSpellID );
		if ( pcModel )
			return pcModel;

		return new cMagicModels( dwSpellID ) ;
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cMagicModels *pcModel );
	static cMagicModels	*FindModel	( DWORD dwSpellID );


	std::string		m_strName;
	DWORD			m_dwSpellID;
	DWORD			m_dwModelNumber;
	WORD			m_wPortalMode;
	WORD			m_wUnknown_1;
	WORD			m_wModel;
	WORD			m_wIcon;
	WORD			m_wParticle;
	WORD			m_wSoundSet;
	DWORD			m_dwFlags1;
	float			m_flScale;
	DWORD			m_dwMedGrey;
	DWORD			m_dwBlueGrey;
	WORD			m_wAssociatedSpell;

	cMagicModels			*m_pcNext;
	cMagicModels			*m_pcPrev;
	std::list< cMagicModels * >	m_lstModels;

private:
	static cMagicModels *m_lpcHashTable[MAGIC_MODEL_TABLE_SIZE];

	static inline Hash_Add( cMagicModels *pcModels )
	{
		const DWORD dwSpellID = pcModels->m_dwSpellID;
		if ( !m_lpcHashTable[dwSpellID] )
			m_lpcHashTable[dwSpellID] = pcModels;
		else
		{
			pcModels->m_pcNext	=	m_lpcHashTable[dwSpellID];
			m_lpcHashTable[dwSpellID]->m_pcPrev	= pcModels;
			m_lpcHashTable[dwSpellID]			= pcModels;
		}
	}

	
	static inline cMagicModels *Hash_Find( DWORD& dwSpellID )
	{
		const DWORD dwModel = dwSpellID;
		cMagicModels *pcModels = m_lpcHashTable[dwModel];
		while ( pcModels )
		{
			if ( pcModels->CompareModel( dwSpellID ) )	return pcModels;
			else	
				pcModels = pcModels->m_pcNext;
		}
		return NULL;
	}
	
	inline BOOL CompareModel( DWORD& dwSpellID )
	{
		if ( dwSpellID == m_dwSpellID )
			return true;
		else
			return false;
	}
};

#endif // !defined(AFX_CMAGICMODELS_H__F2C9124C_F6F0_4FCD_B4FC_1C85BE6113E1__INCLUDED_)