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
 *	@file cClothes.cpp
 *	Implements functionality for clothing.
 *
 *	This class is referenced whenever a piece of clothing is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the message sent for the creation of clothing in the world.
 *
 *	This function is called whenever a piece of clothing should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cClothes::CreatePacket( )
{
	cMessage cmReturn;

	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;

		// the human palette
		if (m_bPaletteChange != 0) 
		{
			cmReturn << WORD( pcModel->m_wUnknown_1 );
		}

		// the palette vectors
		if ( m_bPaletteChange != 0) 
		{
			for (int i = 0; i < m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&m_vectorPal[i],sizeof(m_vectorPal[i]));
			}
		}
		// the texture vectors
		if ( pcModel->m_bTextureChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bTextureChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorTex[i],sizeof(pcModel->m_vectorTex[i]));
			}
		}
		// the model vectors
		if ( pcModel->m_bModelChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bModelChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorMod[i],sizeof(pcModel->m_vectorMod[i]));
			}
		}
	}

	cmReturn.pasteAlign(4);
	cmReturn << pcModel->m_dwFlags1 << 0x414L << 0x65L;

	// MASK 0x00008000 -- Location 
	if ( !m_fIsOwned )
	cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000000L + pcModel->m_wSoundSet;
	cmReturn << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmReturn << 0x34000000 + pcModel->m_dwUnknown_Blue;

	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + pcModel->m_dwModelNumber;
	cmReturn << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmReturn	<< m_wPositionSequence
				<< m_wNumAnims //wNuminteracts 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;
	/*
		Flags2
		DWORD dwFlags2 = 0x81250018;
	*/
	DWORD flags2 = pcModel->m_dwFlags2;
	if (pcObject && pcObject->m_fEquipped == 2)
	{
		flags2 = (flags2 ^ 0x00004000);
		flags2 = (flags2  | 0x00020000 | 0x00008000);
	}
		cmReturn << flags2;
		cmReturn << Name( );					// Object's Name
		cmReturn << pcModel->m_wModel;			// Object's Model
		cmReturn << m_wIcon;					// Object's Icon
		cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
		cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	if(flags2 & 0x00000008)
		cmReturn << pcModel->m_dwValue;
	if(flags2 & 0x00000010)
		cmReturn << pcModel->m_dwUnknown_v2;
	if(flags2 & 0x00000080)
		cmReturn << pcModel->m_dwIconHighlight;
	if(flags2 & 0x00008000)
		cmReturn << pcObject->m_dwOwnerID;
	if(flags2 & 0x00010000)
		cmReturn << pcModel->m_dwEquipPossible;
	if(flags2 & 0x00020000)
		cmReturn << pcModel->m_dwEquipPossible;
	if(flags2 & 0x00040000)
		cmReturn << pcModel->m_dwCoverage;
	if(flags2 & 0x01000000)
		cmReturn << pcModel->m_fWorkmanship;
	if(flags2 & 0x00200000)
		cmReturn << pcModel->m_wBurden;
	if(flags2 & 0x01000000)
		cmReturn << pcModel->m_fWorkmanship;
	if(flags2 & 0x10000000)
		cmReturn << pcModel->m_wHooks;
	if(flags2 & 0x80000000)
		cmReturn << pcModel->m_dwMaterialType;

	return cmReturn;
}

/**
 *	Handles the message sent for the creation of clothing in a container.
 *
 *	This function is called whenever a piece of clothing should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cClothes::CreatePacketContainer(DWORD Container, DWORD ItemModelID)
{
	cMessage cmReturn;

	cItemModels *pcModel = cItemModels::FindModel( ItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;

		// the human palette
		if (m_bPaletteChange != 0) 
		{
			cmReturn << WORD( pcModel->m_wUnknown_1 );
		}

		// the palette vectors
		if ( m_bPaletteChange != 0) 
		{
			for (int i = 0; i < m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&m_vectorPal[i],sizeof(m_vectorPal[i]));
			}
		}
		// the texture vectors
		if ( pcModel->m_bTextureChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bTextureChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorTex[i],sizeof(pcModel->m_vectorTex[i]));
			}
		}
		// the model vectors
		if ( pcModel->m_bModelChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bModelChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorMod[i],sizeof(pcModel->m_vectorMod[i]));
			}
		}
	}

	cmReturn.pasteAlign(4);
	cmReturn << pcModel->m_dwFlags1 << 0x414L << 0x65L;

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000000L + pcModel->m_wSoundSet;
	cmReturn << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmReturn << 0x34000000 + pcModel->m_dwUnknown_Blue;

	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + pcModel->m_dwModelNumber;
	cmReturn << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x0;
	WORD wNumbubbles = 0x0;
	WORD wNumJumps = 0x0;
	WORD wNumOverrides = 0x0;
	WORD wUnkFlag8 = 0x0;
	WORD wUnkFlag10 = 0x0;
	
	cmReturn	<< m_wPositionSequence
				<< m_wNumAnims //wNuminteracts 
				<< wNumbubbles 
				<< wNumJumps 
				<< m_wNumPortals 
				<< m_wNumAnims 
				<< wNumOverrides 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	DWORD flags2 = pcModel->m_dwFlags2;
	if (m_fEquipped == 2)
	{
		flags2 = (flags2 ^ 0x00004000);
		flags2 = (flags2 | 0x00020000 | 0x00008000);
	}
	//can be 0x81254018, 0x81278098
	//	Flags2
	cmReturn << flags2;
	cmReturn << Name( );					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << m_wIcon;					// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	if(flags2 & 0x00000008)
		cmReturn << pcModel->m_dwValue;
	if(flags2 & 0x00000010)
		cmReturn << pcModel->m_dwUnknown_v2;
	if(flags2 & 0x00000080)
		cmReturn << pcModel->m_dwIconHighlight;
	if(flags2 & 0x00004000)
		cmReturn << Container;
	if(flags2 & 0x00008000)
		cmReturn << Container;
	if(flags2 & 0x00010000)
		cmReturn << pcModel->m_dwEquipPossible;
	if(flags2 & 0x00020000)
		cmReturn << pcModel->m_dwEquipPossible;
	if(flags2 & 0x00040000)
		cmReturn << pcModel->m_dwCoverage;
	if(flags2 & 0x01000000)
		cmReturn << pcModel->m_fWorkmanship;
	if(flags2 & 0x00200000)
		cmReturn << pcModel->m_wBurden;
	if(flags2 & 0x01000000)
		cmReturn << pcModel->m_fWorkmanship;
	if(flags2 & 0x10000000)
		cmReturn << pcModel->m_wHooks;
	if(flags2 & 0x80000000)
		cmReturn << pcModel->m_dwMaterialType;

	return cmReturn;
}

/**
 *	Handles the actions of clothing objects.
 *
 *	This function is called whenever a piece of clothing is used or should perform an action.
 */
void cClothes::Action(cClient *who)
{

}

/**
 *	Handles the assessment of clothing objects.
 *
 *	This function is called whenever a piece of clothing is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cClothes::Assess( cClient *pcAssesser )
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	cMessage cmAssess;
	DWORD flags = 0x00000089;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags
	<< 0x01L;				//Success = 0x01L, Failure = 0x00L

	if (flags & 0x00000001 == 0x00000001)
	{
		cmAssess 	<< WORD(0x0003)					//Value count
					<< WORD(0x0010)
					<< 0x13L						//Value key
					<< pcModel->m_dwValue			//Value	
					<< 0x05L						//Burden key
					<< DWORD(pcModel->m_wBurden)	//Burden
					<< 0x1CL						//Armor Level key
					<< pcModel->m_dwArmor_Level;	//Armor Level				
	}
	if (flags & 0x00000002 == 0x00000002)
	{
		cmAssess 	<< WORD(0x0001)					//Value count
					<< WORD(0x0008)
					<< 0x64L						//Dyeable
					<< 0x01L;						//True = 0x01L, False = 0x00L
	}
	if (flags & 0x00000008 == 0x00000008)
	{
		cmAssess	<< WORD(0x0001)					//Count (as above) total number of DWORDS
					<< WORD(0x0008)					//Unknown
					<< 0x10L						//Full Description key
					<< this->m_strName.c_str();		//Description
	}
	if (flags & 0x00000080 == 0x00000080)
	{
		cmAssess	<< pcModel->m_fProt_Slashing	//Slashing
					<< pcModel->m_fProt_Piercing	//Piercing			
					<< pcModel->m_fProt_Bludgeon	//Bludgeon
					<< pcModel->m_fProt_Fire		//Fire
					<< pcModel->m_fProt_Cold		//Cold
					<< pcModel->m_fProt_Acid		//Acid
					<< pcModel->m_fProt_Electric;	//Electric
	}

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}