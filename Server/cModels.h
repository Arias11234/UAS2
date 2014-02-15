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
 *	@file cModels.h
 */

#if !defined(AFX_CMODELS_H__08D0B2E9_9DDB_4F52_B2A8_8847149A4EAB__INCLUDED_)
#define AFX_CMODELS_H__08D0B2E9_9DDB_4F52_B2A8_8847149A4EAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <winsock2.h>
#include "shared.h"

#define MONSTER_MODEL_TABLE_SIZE 3000

class cModels  
{
	friend class cMasterServer;
public:
	cModels( DWORD dwModelID, BOOL fAddToHash = TRUE )
			:	m_dwModelID( dwModelID ),
				m_pcPrev		( NULL ),
				m_pcNext		( NULL )
	{
		m_dwUnknownDword	=	0x0L;
		m_dwUnknown			=	0x0L;
		m_wSeagreen10		=	0x0;
		m_dwFlags2			=	0x00800016;
		m_dwObjectFlags1	=	0x00000010;
		m_dwObjectFlags2	=	0x00000014;
		m_bItemSlots		=	0xFF;
		m_bPackSlots		=	0xFF;
		m_wUnknown1			=	0x0;
		m_flScale			=	1.0f;
		m_dwTrio1[0]		=	0x0;
		m_dwTrio1[1]		=	0x0;
		m_dwTrio1[2]		=	0x0;
		m_dwTrio2[0]		=	0x0;
		m_dwTrio2[1]		=	0x0;
		m_dwTrio2[2]		=	0x0;
		m_dwTrio3[0]		=	0x0;
		m_dwTrio3[1]		=	0x0;
		m_dwTrio3[2]		=	0x0;
		m_dwSpecies			=	0x0;

		if( fAddToHash )
		Hash_Add( this );
	}
	virtual ~cModels()
	{
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[m_dwModelID] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}
		
	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline cModels *Hash_New( DWORD& dwModelID )
	{
		cModels *pcModel = Hash_Find( dwModelID );
		if ( pcModel )
			return pcModel;

		return new cModels( dwModelID ) ;
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cModels *pcModel );
	static cModels	*FindModel	( DWORD dwModelID );

	std::string		m_strName;
	std::string		m_strDescription;
	DWORD			m_dwSpecies;
	DWORD			m_dwModelID;
	
	// Model Vectors
	BYTE			m_bPaletteChange;
	DWORD			m_wPaletteVector;
	sPaletteChange	m_vectorPal[255];

	BYTE			m_bTextureChange;
	DWORD			m_wTextureVector;
	sTextureChange	m_vectorTex[255];
		
	BYTE			m_bModelChange;
	DWORD			m_wModelVector;
	sModelChange	m_vectorMod[255];

	WORD			m_wPortalMode;
	WORD			m_wUnknown_1;
	
	DWORD			m_dwUnknownCount;
	BYTE			m_bInitialAnimation[200];
	DWORD			m_dwUnknownDword;
	DWORD			m_dwUnknown;
	WORD			m_wSeagreen8;
	WORD			m_wSeagreen10;

	WORD			m_wModel;
	WORD			m_wIcon;
	WORD			m_wAnimConfig;
	WORD			m_wSoundSet;
	
	DWORD			m_dwModelNumber;
	DWORD			m_dwObjectFlags1;
	DWORD			m_dwObjectFlags2;
	BYTE			m_bItemSlots;
	BYTE			m_bPackSlots;
	DWORD			m_dwFlags1;
	DWORD			m_dwFlags2;
	WORD			m_wUnknown1;
	DWORD			m_dwUnknown_Blue;
	float			m_flScale;
	DWORD			m_dwUnknown_LightGrey;
	DWORD			m_dwTrio1[3];
	DWORD			m_dwTrio2[3];
	DWORD			m_dwTrio3[3];
	DWORD			m_dwMedGrey;
	DWORD			m_dwBlueGrey;
	DWORD			m_dwUnknown_v2;
	DWORD			m_dwUnknown_v6; 
	
	int				m_isContainer;

	//Stats for damage
	float			mob_strength;

	cAnimates		m_cAnimations;

	cModels			*m_pcNext;
	cModels			*m_pcPrev;
	std::list< cModels * >	m_lstModels;

private:
	static cModels *m_lpcHashTable[MONSTER_MODEL_TABLE_SIZE];

	static inline Hash_Add( cModels *pcModels )
	{
		const DWORD dwModelID = pcModels->m_dwModelID;
		if ( !m_lpcHashTable[dwModelID] )
			m_lpcHashTable[dwModelID] = pcModels;
		else
		{
			pcModels->m_pcNext	=	m_lpcHashTable[dwModelID];
			m_lpcHashTable[dwModelID]->m_pcPrev	= pcModels;
			m_lpcHashTable[dwModelID]			= pcModels;
		}
	}
	
	static inline cModels *Hash_Find( DWORD& dwModelID )
	{
		const DWORD dwModel = dwModelID;
		cModels *pcModels = m_lpcHashTable[dwModel];
		while ( pcModels )
		{
			if ( pcModels->CompareModel( dwModelID ) )	return pcModels;
			else	
				pcModels = pcModels->m_pcNext;
		}
		return NULL;
	}
	
	inline BOOL CompareModel( DWORD& dwModelID )
	{
		if ( dwModelID == m_dwModelID )
			return true;
		else
			return false;
	}
};

#endif // !defined(AFX_CMODELS_H__08D0B2E9_9DDB_4F52_B2A8_8847149A4EAB__INCLUDED_)