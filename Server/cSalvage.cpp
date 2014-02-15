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
 *	@file cSalvage.cpp
 *	Implements functionality for salvage
 *
 *	This class is referenced whenever salvage is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the message sent for the creation of salvage in the world.
 *
 *	This function is called whenever salvage should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cSalvage::CreatePacket()
{
	cMessage cmReturn;
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< pcModel->m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;

		// The Model Vectors
		if ( pcModel->m_bPaletteChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorPal[i],sizeof(pcModel->m_vectorPal[i]));
			}
		}
		
		if (pcModel->m_bPaletteChange != 0) 
		{
			cmReturn << WORD( pcModel->m_wUnknown1 );
		}
		
		if ( pcModel->m_bTextureChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bTextureChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorTex[i],sizeof(pcModel->m_vectorTex[i]));
			}
		}

		if ( pcModel->m_bModelChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bModelChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorMod[i],sizeof(pcModel->m_vectorMod[i]));
			}
		}
	}

	cmReturn.pasteAlign(4);
	pcModel->m_dwFlags1 += 0x00008000;

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

	DWORD dwFlags2 = pcModel->m_dwFlags2 -= 0x00004000;

	cmReturn << dwFlags2;
	char Salvage[100];
	sprintf(Salvage,"%s (%d) ",Name(),pcModel->m_wUses);
	cmReturn << Salvage;					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << this->m_wIcon;				// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	if(pcModel->m_dwFlags2 & 0x00000008)
		cmReturn << pcModel->m_dwValue;
	if(pcModel->m_dwFlags2 & 0x00000010)
		cmReturn << 0x00080008;
	if(pcModel->m_dwFlags2 & 0x00080000)
		cmReturn << pcModel->m_dwUseableOn;
	if(pcModel->m_dwFlags2 & 0x00000400)
		cmReturn << pcModel->m_wUses;		//For salvage, this is quantity
	if(pcModel->m_dwFlags2 & 0x00000800)
		cmReturn << WORD(0x0064);			//For salvage this is max quantity (always 100)
	if(pcModel->m_dwFlags2 & 0x00001000)	
		cmReturn << WORD(0x0001);			//Always 1
	if(pcModel->m_dwFlags2 & 0x00002000)	
		cmReturn << WORD(0x0001);			//Always 1
	if(pcModel->m_dwFlags2 & 0x00200000)	
		cmReturn << pcModel->m_wBurden;
	if(pcModel->m_dwFlags2 & 0x01000000)
		cmReturn << pcModel->m_fWorkmanship;
	if(pcModel->m_dwFlags2 & 0x10000000)
		cmReturn << pcModel->m_wHooks;
	if(pcModel->m_dwFlags2 & 0x40000000)
		cmReturn << 0x0L;
	if(pcModel->m_dwFlags2 & 0x80000000)
		cmReturn << pcModel->m_dwMaterialType;

	return cmReturn;
}

/**
 *	Handles the message sent for the creation of salvage in a container.
 *
 *	This function is called whenever salvage should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cSalvage::CreatePacketContainer(DWORD Container, DWORD ItemModelID)
{
	cMessage cmReturn;
	cItemModels *pcModel = cItemModels::FindModel( ItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
		cmReturn	<< pcModel->m_bPaletteChange
					<< pcModel->m_bTextureChange
					<< pcModel->m_bModelChange;

		// The Model Vectors
		if ( pcModel->m_bPaletteChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorPal[i],sizeof(pcModel->m_vectorPal[i]));
			}
		}
			cmReturn << WORD( 0x0000 );

		if ( pcModel->m_bTextureChange != 0) 
		{
			for (int i = 0; i < pcModel->m_bTextureChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&pcModel->m_vectorTex[i],sizeof(pcModel->m_vectorTex[i]));
			}
		}

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
//	if ( !m_fIsOwned )
//	cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );

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
	
	//	Flags2
	//	DWORD dwFlags2 = 0xD1287C18;
	cmReturn << pcModel->m_dwFlags2;		// Object Flags
	char Salvage[100];
	sprintf(Salvage,"%s (%d) ",Name(),pcModel->m_wUses);
	cmReturn << Salvage;					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << this->m_wIcon;				// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

	if(pcModel->m_dwFlags2 & 0x00000008)
		cmReturn << pcModel->m_dwValue;
	if(pcModel->m_dwFlags2 & 0x00000010)
		cmReturn << 0x00080008;
	if(pcModel->m_dwFlags2 & 0x00080000)
		cmReturn << pcModel->m_dwUseableOn;
	if(pcModel->m_dwFlags2 & 0x00000400)
		cmReturn << pcModel->m_wUses;		//For salvage, this is quantity
	if(pcModel->m_dwFlags2 & 0x00000800)
			cmReturn << WORD(0x0064);			//For salvage this is max quantity (always 100)
	if(pcModel->m_dwFlags2 & 0x00001000)	
		cmReturn << WORD(0x0001);			//Always 1
	if(pcModel->m_dwFlags2 & 0x00002000)	
		cmReturn << WORD(0x0001);			//Always 1
	if(pcModel->m_dwFlags2 & 0x00004000)	
		cmReturn << Container;
	if(pcModel->m_dwFlags2 & 0x00200000)	
		cmReturn << pcModel->m_wBurden;
	if(pcModel->m_dwFlags2 & 0x01000000)
		cmReturn << pcModel->m_fWorkmanship;
	if(pcModel->m_dwFlags2 & 0x10000000)
		cmReturn << pcModel->m_wHooks;
	if(pcModel->m_dwFlags2 & 0x40000000)
		cmReturn << 0x0L;
	if(pcModel->m_dwFlags2 & 0x80000000)
		cmReturn << pcModel->m_dwMaterialType;

	return cmReturn;
}

/**
 *	Handles the actions of salvage objects.
 *
 *	This function is called whenever salvage is used or should perform an action.
 */
void cSalvage::Action(cClient* who)
{

}

/**
 *	Handles the assessment of salvage objects.
 *
 *	This function is called whenever salvage is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cSalvage::Assess(cClient *pcAssesser)
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	cMessage cmAssess;
	DWORD flags = 0x00000009;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags 
	<< 0x01L						//Success = 0x01, Failure = 0x00
	<< WORD(0x0006)					//Total number of DWORDS
	<< WORD(0x0010)					//Unknown
	<< 0x21L						// Bonded
	<< 0x01L
	<< 0x83L						//Material
	<< pcModel->m_dwMaterialType
	<< 0x13L						//Value
	<< pcModel->m_dwValue			// 4 pyreals
	<< 0x05L						//Burden
	<< DWORD (pcModel->m_wBurden)	//50 bu
	<< 0x69L						//Workmanship					
	<< pcModel->m_fWorkmanship						
	<< 0xAAL						//Number of Items salvaged from (TODO: Implement this :p)
	<< 0x01L

    << WORD(0x0002)					//Count (as above) total number of DWORDS
	<< WORD(0x0008)					//Unknown
	<< 0x0EL						//String
	<< pcModel->m_strDescription.c_str()
	<< 0x0FL
	<< "Test";						

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);	
}