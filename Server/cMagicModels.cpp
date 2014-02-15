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
 *	@file cMagicModels.cpp
 *	Implements functionality for the cMagicModels class.
 *
 *	Author: G70mb2
 */

#include "cMagicModels.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAGIC_MODEL_TABLE_SIZE 1000

cMagicModels *cMagicModels::m_lpcHashTable[MAGIC_MODEL_TABLE_SIZE];

/**
 *	Removes all magic models from the magic model hash list.
 */
void cMagicModels::Hash_Erase()
{
	cMagicModels *pcModels, *pcPrevModel;
	for ( int i = 0; i < MAGIC_MODEL_TABLE_SIZE; ++i )
	{
		pcModels = m_lpcHashTable[i];
		while ( pcModels )
		{
			pcPrevModel	= pcModels;
			pcModels	= pcModels->m_pcNext;

			Hash_Remove( pcPrevModel );
		}
	}
}

/**
 *	Removes a magic model from the magic model hash list.
 *
 *	@param *pcModel - A pointer to the magic model to be removed.
 */
void cMagicModels::Hash_Remove( cMagicModels *pcModel )
{
	SAFEDELETE( pcModel )
}

/**
 *	Finds a magic model.
 *
 *	This function is called when a particular magic model needs to be found.
 *	The search is performed by searching for the magic model's spell ID.
 *
 *	@param dwSpellID - The magic model's spell ID.
 *
 *	@return *cMagicModels - A pointer to the magic model.
 */
cMagicModels *cMagicModels::FindModel( DWORD dwSpellID )
{
	cMagicModels *pcModel;
	for ( int i = 0; i < MAGIC_MODEL_TABLE_SIZE; ++i )
	{
		pcModel = m_lpcHashTable[i];
		while ( pcModel )
		{
			if ( pcModel->m_dwSpellID )
			{
				if ( dwSpellID == pcModel->m_dwSpellID )
					return pcModel;
			}
			pcModel = pcModel->m_pcNext;
		}
	}
	return NULL;
}