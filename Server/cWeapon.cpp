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
 *	@file cWeapon.cpp
 *	Implements functionality for weapons.
 *
 *	This class is referenced whenever a weapons is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the message sent for the creation of weapons in the world.
 *
 *	This function is called whenever a weapon should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cWeapon::CreatePacket( )
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
			//Cubem0j0:  Test code for armor only.
			cmReturn << WORD( 0x0C50 );
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

	cmReturn << 0x00029801 << 0x414L << 0x65L;

	// MASK 0x00008000 -- Location 
	if ( !m_fIsOwned )
	cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000014L + pcModel->m_wSoundSet;
	cmReturn << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmReturn << 0x3400002B + pcModel->m_dwUnknown_Blue;

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

	// Flags2
	// DWORD dwFlags2 = 0x91210298;

	cmReturn << pcModel->m_dwFlags2;
	cmReturn << Name( );					// Objects Name
	cmReturn << pcModel->m_wModel;			// Objects Model
	cmReturn << pcModel->m_wIcon;			// Objects Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;

	// Masked against dwFlags2
	// Mask 0x0008 - Value
	if(pcModel->m_dwFlags2 & 0x00000008)
	{
		cmReturn << pcModel->m_dwValue;
	}
	// Mask 0x0010 dwUnknown_v2
	if(pcModel->m_dwFlags2 & 0x00000010)
	{
		cmReturn << pcModel->m_dwUnknown_v2;
	}
	// Mask 0x0080 Icon Highlight
	if(pcModel->m_dwFlags2 & 0x00000080)
	{
		cmReturn << pcModel->m_dwIconHighlight;
	}
	// Mask 0x0200 Wield Type
	if(pcModel->m_dwFlags2 & 0x00000200)
	{
		cmReturn << pcModel->m_bWieldType;
	}
	// Mask 0x1000 Stack
	if(pcModel->m_dwFlags2 & 0x00001000)
	{
		cmReturn << pcModel->m_wStack;
	}
	// Mask 0x2000 Stack Limit
	if(pcModel->m_dwFlags2 & 0x00002000)
	{
		cmReturn << pcModel->m_wStackLimit;
	}
	// Mask 0x10000 Equip Possible
	if(pcModel->m_dwFlags2 & 0x00010000)
	{
		cmReturn << DWORD(0x100000);
	//cmReturn << pcModel->m_dwEquipPossible;
	}
	// Mask 0x00200000 - Burden
	if(pcModel->m_dwFlags2 & 0x00200000)
	{
		cmReturn << pcModel->m_wBurden;
	}
	// Mask 0x01000000 - Workmanship
	if(pcModel->m_dwFlags2 & 0x01000000)
	{
		cmReturn << pcModel->m_fWorkmanship;
	}
	// Mask 0x10000000 - Hookable
	if(pcModel->m_dwFlags2 & 0x10000000)
	{
		cmReturn << pcModel->m_wHooks;
	}
	// Mask 0x80000000 - Material
	cmReturn << WORD(0x0000);
	cmReturn << BYTE(0x00);
/*
	// Masked against dwFlags2
	// Mask 0x0008 - Value
	cmReturn << DWORD(0x88);
	// Mask 0x0010 dwUnknown_v2
	cmReturn << DWORD(0x00000001);
	// Mask 0x0080 Icon Highlight
	cmReturn << DWORD(0x40);
	// Mask 0x0200 Wield Type
	cmReturn << BYTE(0x01);
	// Mask 0x10000 Equip Possible
		cmReturn << DWORD(0x100000);
	// Mask 0x00200000 - Burden
		cmReturn << WORD(0x0040);
	// Mask 0x01000000 - Workmanship
	cmReturn << float(4.0);
	// Mask 0x10000000 - Hookable
	cmReturn << WORD(0x0001);
	// Mask 0x80000000 - Material
	cmReturn << DWORD(0x33);
*/
	return cmReturn;
}

/**
 *	Handles the message sent for the creation of weapons in a container.
 *
 *	This function is called whenever a weapon should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cWeapon::CreatePacketContainer( DWORD Container, DWORD ItemModelID )
{
	cMessage cmReturn;
	cItemModels *pcModel = cItemModels::FindModel( ItemModelID );
	cObject *pcObject = cWorldManager::FindObject( m_dwGUID );

	if( pcModel )
	{
		cmReturn	<< 0xF745L << m_dwGUID << BYTE(0x11) //0x11 is a constant
					<< pcModel->m_bPaletteChange
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
	
	cmReturn << 0x00021801L << 0x414L << 0x65L;


	// MASK 0x00008000 -- Location 
//	if ( !m_fIsOwned )
//	cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );

	// MASK 0x00000800 -- Sound Set 
	DWORD dwSoundSet = 0x20000014L;
	cmReturn << dwSoundSet;

	// MASK 0x00001000 -- Particle Effects (unknown_blue)
	cmReturn << 0x3400002BL;

	// MASK 0x00000001 -- ModelNumber
	DWORD dwModel = 0x02000000L + pcModel->m_dwModelNumber;
	cmReturn << dwModel;

	// SeaGreens
	WORD wNuminteracts = 0x1;
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
	
	// Flags2
	// DWORD dwFlags2 = 0x91214298;

	cmReturn << pcModel->m_dwFlags2;
	cmReturn << Name( );					// Objects Name
	cmReturn << pcModel->m_wModel;			// Objects Model
	cmReturn << pcModel->m_wIcon;			// Objects Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;

	// Masked against dwFlags2
	// Mask 0x0008 - Value
	if(pcModel->m_dwFlags2 & 0x00000008)
	{
		cmReturn << pcModel->m_dwValue;
	}
	// Mask 0x0010 dwUnknown_v2
	if(pcModel->m_dwFlags2 & 0x00000010)
	{
		cmReturn << pcModel->m_dwUnknown_v2;
	}
	// Mask 0x0080 Icon Highlight
	if(pcModel->m_dwFlags2 & 0x00000080)
	{
		cmReturn << pcModel->m_dwIconHighlight;
	}
	// Mask 0x0200 Wield Type
	if(pcModel->m_dwFlags2 & 0x00000200)
	{
		cmReturn << pcModel->m_bWieldType;
	}
	// Mask 0x1000 Stack
	if(pcModel->m_dwFlags2 & 0x00001000)
	{
		cmReturn << pcModel->m_wStack;
	}
	// Mask 0x2000 Container
	if(pcModel->m_dwFlags2 & 0x00002000)
	{
		cmReturn << pcModel->m_wStackLimit;
	}
	// Mask 0x4000 Container
	if(pcModel->m_dwFlags2 & 0x00004000)
	{
		cmReturn << Container;
	}
	// Mask 0x10000 Equip Possible
	if(pcModel->m_dwFlags2 & 0x00010000)
	{
		cmReturn << DWORD(0x100000);
	//cmReturn << pcModel->m_dwEquipPossible;
	}
	// Mask 0x00200000 - Burden
	if(pcModel->m_dwFlags2 & 0x00200000)
	{
		cmReturn << pcModel->m_wBurden;
	}
	// Mask 0x01000000 - Workmanship
	if(pcModel->m_dwFlags2 & 0x01000000)
	{
		cmReturn << pcModel->m_fWorkmanship;
	}
	// Mask 0x10000000 - Hookable
	if(pcModel->m_dwFlags2 & 0x10000000)
	{
		cmReturn << pcModel->m_wHooks;
	}
	// Mask 0x80000000 - Material
	cmReturn << WORD(0x0000);
	cmReturn << BYTE(0x00);

	return cmReturn;
}

/**
 *	Handles the actions of weapons objects.
 *
 *	This function is called whenever a weapon is used or should perform an action.
 */
void cWeapon::Action(cClient *who)
{

}

/**
 *	Handles the assessment of weapons objects.
 *
 *	This function is called whenever a weapon is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cWeapon::Assess( cClient *pcAssesser )
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	
	cMessage cmAssess;
	DWORD flags = 0x0000002D;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags << 0x01L << WORD(0x03) << WORD(0x10)
	<< 0x13L		//Value
	<< this->m_dwValue
	<< 0x05L		//Burden
	<< DWORD(this->m_wWeight)
	<< 0x2FL		//??
	<< 0x01L
	<< WORD(0x01)
	<< WORD(0x08)
	<< 0x1DL
	<< 0x00L
	<< 0x00L
	<< 0x00L		
	<< this->m_dwDamageType		//Damage type
	<< this->m_dwWeaponSpeed		//Speed
	<< this->m_dwWeaponSkill		//Skill
	<< this->m_dwWeaponDamage		//max damage
	<< this->m_dWeaponVariance	//variance
	<< double(1.0)
	<< double(1.0)
	<< double(0.2);
	if(this->m_dWeaponModifier > 1.1)
		cmAssess << this->m_dWeaponModifier;	//defense mod	(1.0  = 0%)
	if(this->m_dWeaponAttack > 1.1)
		cmAssess << this->m_dWeaponAttack;	//attack mod	(1.0  = 0%)
	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}