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
 *	@file cItemModels.h
 */

#define ITEM_MODEL_TABLE_SIZE 4000

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <winsock2.h>
#include "shared.h"

class cItemModels  
{
	friend class cMasterServer;
	friend class cObject;
	friend class cWeapon;

public:
	cItemModels( DWORD dwModelID, BOOL fAddToHash = TRUE )
			:	m_dwModelID( dwModelID ),
				m_pcPrev		( NULL ),
				m_pcNext		( NULL )
	{
		
		if( fAddToHash )
		Hash_Add( this );

	}
	virtual ~cItemModels()
	{
		if ( m_pcPrev )	m_pcPrev->m_pcNext = m_pcNext;
		else			m_lpcHashTable[m_dwModelID] = m_pcNext;
		if ( m_pcNext )	m_pcNext->m_pcPrev = m_pcPrev;
	}
		
	static inline void Hash_Load( ) 
	{ 
		ZeroMemory( m_lpcHashTable, sizeof( m_lpcHashTable ) ); 
	}
	
	static inline cItemModels *Hash_New( DWORD& dwModelID )
	{
		cItemModels *pcModel = Hash_Find( dwModelID );
		if ( pcModel )
			return pcModel;

		return new cItemModels( dwModelID ) ;
	}

	static void		Hash_Erase	( );
	static void		Hash_Remove	( cItemModels *pcModel );
	static cItemModels	*FindModel	( DWORD dwModelID );

	bool			m_clothingModelLoaded;
	
	std::string		m_strName;
	std::string		m_strDescription;
	DWORD			m_PortalLinker;
	DWORD			m_dwModelID;
	
	// Model vectors
	BYTE			m_bPaletteChange;
	DWORD			m_wPaletteVector;
	sPaletteChange	m_vectorPal[255];

	BYTE			m_bTextureChange;
	DWORD			m_wTextureVector;
	sTextureChange	m_vectorTex[255];
		
	BYTE			m_bModelChange;
	DWORD			m_wModelVector;
	sModelChange	m_vectorMod[255];
	
	// Model clothing vectors
	BYTE			m_bWearPaletteChange;
	DWORD			m_wWearPaletteVector;
	sPaletteChange	m_WearVectorPal[255];

	BYTE			m_bWearTextureChange;
	DWORD			m_wWearTextureVector;
	sTextureChange	m_WearVectorTex[255];
		
	BYTE			m_bWearModelChange;
	DWORD			m_wWearModelVector;
	sModelChange	m_WearVectorMod[255];

	WORD			m_wPortalMode;
	WORD			m_wUnknown_1;
	
	DWORD			m_dwUnknownCount;
	DWORD			m_dwUnknownDword;
	DWORD			m_dwUnknown;
	WORD			m_wSeagreen8;
	WORD			m_wSeagreen10;

	WORD			m_wModel;
	WORD			m_wIcon;
	WORD			m_wSoundSet;
	
	DWORD			m_dwModelNumber;
	DWORD			m_dwObjectFlags1;
	DWORD			m_dwObjectFlags2;
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
	
	//Item specific values
	DWORD			m_dwValue;
	DWORD			m_dwUseableOn;
	float			m_fApproachDistance;
	DWORD			m_dwIconHighlight;
	WORD			m_wAmmoType;
	BYTE			m_bWieldType;
	WORD			m_wUses;
	WORD			m_wUseLimit;
	WORD			m_wStack;
	WORD			m_wStackLimit;
	DWORD			m_dwContainerID;
	DWORD			m_dwVitalID;
	DWORD			m_dwEquipPossible;
	DWORD			m_dwEquipActual;
	DWORD			m_dwCoverage;
	float			m_fWorkmanship;
	WORD			m_wBurden;
	DWORD			m_dwSpellID;
	//For scrolls
	WORD			m_wSpellID;
	DWORD			m_dwOwner;
	WORD			m_wHooks;
	DWORD			m_dwMaterialType;
	DWORD			m_dwQuestItemID;
	UINT			m_vital_affect;
	int				m_ItemType;

	//Armor
	DWORD		m_dwArmor_Level;
	float		m_fProt_Slashing;
	float		m_fProt_Piercing;
	float		m_fProt_Bludgeon;
	float		m_fProt_Fire;
	float		m_fProt_Cold;
	float		m_fProt_Acid;
	float		m_fProt_Electric;

	//Weapons
	DWORD	m_dwWeaponDamage;
	DWORD	m_dwWeaponSpeed;
	DWORD	m_dwWeaponSkill;
	DWORD	m_dwDamageType;

	double	m_dWeaponVariance;
	double	m_dWeaponModifier;
	double	m_dWeaponPower;
	double	m_dWeaponAttack;

	int		m_isContainer;
	int		m_isClothing;
	int		m_isUAWeapon;

	//Books
	DWORD		m_ContentPages;
	DWORD		m_UsedPages;
	DWORD		m_TotalPages;
	std::string m_Author;
	std::string m_Title;
	std::string m_Comment;
	std::string m_CommentAuthor;

typedef struct Pages
{
	std::string textPages;
};

Pages m_Pages[50];
	/*
	std::string m_Page1;
	std::string m_Page2;
	std::string m_Page3;
	std::string m_Page4;
	std::string m_Page5;
	std::string m_Page6;
	std::string m_Page7;
	std::string m_Page8;
	std::string m_Page9;
	*/

	// Animation Arrays

	cItemModels			*m_pcNext;
	cItemModels			*m_pcPrev;
	std::list< cItemModels * >	m_lstModels;

private:
	static cItemModels *m_lpcHashTable[ITEM_MODEL_TABLE_SIZE];

	static inline Hash_Add( cItemModels *pcItemModels )
	{
		const DWORD dwModelID = pcItemModels->m_dwModelID;
		if ( !m_lpcHashTable[dwModelID] )
			m_lpcHashTable[dwModelID] = pcItemModels;
		else
		{
			pcItemModels->m_pcNext	=	m_lpcHashTable[dwModelID];
			m_lpcHashTable[dwModelID]->m_pcPrev	= pcItemModels;
			m_lpcHashTable[dwModelID]			= pcItemModels;
		}
	}

	static inline cItemModels *Hash_Find( DWORD& dwModelID )
	{
		const DWORD dwModel = dwModelID;
		cItemModels *pcItemModels = m_lpcHashTable[dwModel];
		while ( pcItemModels )
		{
			if ( pcItemModels->CompareModel( dwModelID ) )	return pcItemModels;
			else	
				pcItemModels = pcItemModels->m_pcNext;
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