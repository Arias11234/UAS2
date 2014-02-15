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
 *	@file cNPCModels.cpp
 *	Implements functionality for NPC models.
 *
 *	An NPC model consititues the palette, texture, and model information for an NPC.
 *
 *	Author: Cube
 */

#include "cNPCModels.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define NPC_MODEL_TABLE_SIZE 2000

cNPCModels *cNPCModels::m_lpcHashTable[NPC_MODEL_TABLE_SIZE];

/**
 *	Erases all NPC models from the NPC model hash list.
 */
void cNPCModels::Hash_Erase()
{
	cNPCModels *pcModels, *pcPrevModel;
	for ( int i = 0; i < NPC_MODEL_TABLE_SIZE; ++i )
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
 *	Removes an NPC model from the NPC model hash list.
 *
 *
 *	@param *pcModel - A pointer to the NPC model to be removed.
 */
void cNPCModels::Hash_Remove( cNPCModels *pcModel )
{
	SAFEDELETE( pcModel )
}

/**
 *	Finds an NPC model.
 *
 *	This function is called when a particular NPC model needs to be found.
 *	The search is performed by searching for the NPC model's model ID.
 *
 *	@param dwModelID - The NPC model's model ID.
 *
 *	@return *cNPCModels - A pointer to the NPC model.
 */
cNPCModels *cNPCModels::FindModel( DWORD dwModelID )
{
	cNPCModels *pcModel;
	for ( int i = 0; i < NPC_MODEL_TABLE_SIZE; ++i )
	{
		pcModel = m_lpcHashTable[i];
		while ( pcModel )
		{
			if ( pcModel->m_dwModelID )
			{
				if ( dwModelID == pcModel->m_dwModelID )
					return pcModel;
			}
			pcModel = pcModel->m_pcNext;
		}
	}
	return NULL;
}