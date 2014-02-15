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
 *	@file cNPCModels.h
 *
 *	Author: Cubem0j0
 */

#ifndef CNPCMODELS_H__
#define CNPCMODELS_H__

#include "shared.h"

#define NPC_MODEL_TABLE_SIZE 2000

class cNPCModels  
{
	friend class cMasterServer;
public:
	cNPCModels( DWORD dwModelID, BOOL fAddToHash = TRUE )
			:	m_dwModelID( dwModelID ),
				m_pcPrev		( NULL ),
				m_pcNext		( NULL )
	{
		if( fAddToHash )
		Hash_Add( this );
	}
	virtual ~cNPCModels()
	{
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[m_dwModelID] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}
		
	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline cNPCModels *Hash_New( DWORD& dwModelID )
	{
		cNPCModels *pcModel = Hash_Find( dwModelID );
		if ( pcModel )
			return pcModel;

		return new cNPCModels( dwModelID ) ;
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cNPCModels *pcModel );
	static cNPCModels	*FindModel	( DWORD dwModelID );


	std::string		m_strName;
	std::string		m_strDescription;
	
	WORD			m_wPaletteCode;
	
	DWORD			m_dwModelID;
	
	float			m_flScale;
	
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

	WORD			m_wModel;
	WORD			m_wIcon;
	DWORD			m_dwModelNumber;
	
	cAnimates		m_cAnimations;

	cNPCModels			*m_pcNext;
	cNPCModels			*m_pcPrev;
	std::list< cNPCModels * >	m_lstNPCModels;

private:
	static cNPCModels *m_lpcHashTable[NPC_MODEL_TABLE_SIZE];

	static inline Hash_Add( cNPCModels *pcModels )
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

	static inline cNPCModels *Hash_Find( DWORD& dwModelID )
	{
		const DWORD dwModel = dwModelID;
		cNPCModels *pcModels = m_lpcHashTable[dwModel];
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

#endif