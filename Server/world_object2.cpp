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
 *	@file world_object2.cpp
 *	Implements functionality for merchant signs.
 *	Inherits from the cObject class.
 *
 *	Author: Cubem0j0
 */

#include "avatar.h"
#include "object.h"
#include "cWObjectModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the creation of merchant signs.
 *
 *	Called whenever a merchant sign object should be initialized.
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cMerchantSign::CreatePacket()
{
	cMessage cmReturn;
	cWObjectModels *pcModel = cWObjectModels::FindModel( m_dwWOModelID );
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
	
	if(iObjectType == 0)
	//Cube: Everything else.
	{
		cmReturn << pcModel->m_dwFlags1 << WORD(0x0418) << WORD(0x0001);
	}

	//Cube:  Readable signs
	if(iObjectType == 1)
	{
		cmReturn << pcModel->m_dwFlags1 << 0x0418L << 0x0065L;
	}

	//Cube:  Nullified Statues
	if(iObjectType == 2)
	{
		cmReturn << pcModel->m_dwFlags1 << 0x0418L; //<< 0x0008L;
	}

	//Cubem0j0:  This was added for Gharundhim signs, if this was not added they would fall to the ground.
	if(iObjectType == 3)
	{
		cmReturn << pcModel->m_dwFlags1 << WORD(0x0018) << WORD(0x0000);
	}

	if (pcModel->m_dwFlags1 &  0x00010000)
	{
		cmReturn << pcModel->m_dwUnknownCount;// pcModel->m_dwUnknownCount;
		// Vector of UnknownCount
		for ( int i = 0;i < pcModel->m_dwUnknownCount; i++) 
		{
			cmReturn << pcModel->m_bInitialAnimation[i]; //pcModel->m_bInitialAnimation[i]; 
		}
		cmReturn << pcModel->m_dwUnknownDword;
	}
	// Flags1 &  0x00020000
	if (pcModel->m_dwFlags1 &  0x00020000)
	{
		cmReturn << pcModel->m_dwUnknown;
	}
	// Flags1 &  0x00008000 - Location Data
	if (pcModel->m_dwFlags1 &  0x00008000)
	{
		cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
	}
	// Flags1 &  0x00000002 - Animation Config
	if (pcModel->m_dwFlags1 &  0x00000002)
	{
		//Format = 0x09000000
		cmReturn << 0x09000000 + pcModel->m_wAnimConfig;
	}
	// Flags1 &  0x00000800 - sound Set
	if (pcModel->m_dwFlags1 &  0x00000800)
	{
		//Format = 0x20000001;
		cmReturn << 0x20000000 + pcModel->m_wSoundSet;
	}
	// Flags1 & 0x00001000 - Particle Effects ?
	if (pcModel->m_dwFlags1 & 0x00001000)
	{
		//Format = 0x34000004;
		cmReturn << 0x34000000 + pcModel->m_dwUnknown_Blue;
	}
	// Flags1 & 0x00000001 - Object's Model Number
	if (pcModel->m_dwFlags1 & 0x00000001)
	{
		//Format = 0x02000001; 
		cmReturn << 0x02000000 + pcModel->m_dwModelNumber;
	}
	// Flags1 & 0x00000080 - Unknown_Green This represents Model Scale 
	if (pcModel->m_dwFlags1 & 0x00000080)
	{
		if(pcModel->m_flScale == 0)
		{
			cmReturn << 1.0f;
		}
		else
		{
			cmReturn << pcModel->m_flScale; // Scale?
		}
	}
	// MASK 0x00008000 - Location 
//	if ( !m_fIsOwned )
//		cmReturn.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );


	// MASK 0x00000001 ModelNumber
//	DWORD dwModel = 0x02000000L + pcModel->m_dwModelNumber;
//	cmReturn << dwModel;

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
	
	switch (iObjectType)
	{
		default:
			{
				DWORD dwFlags2 = 0x00200018;
		
				cmReturn << dwFlags2 << m_strName.c_str( ) << WORD(pcModel->m_wModel) << WORD(pcModel->m_wIcon);

				DWORD dwObjectFlags1 = 0x80L;	
				cmReturn << dwObjectFlags1;

				DWORD dwObjectFlags2 = 0x14L;
				cmReturn << dwObjectFlags2;
				// Masked against dwFlags2
				//Mask 0x0008 - Value
				cmReturn << DWORD(0x00);
				// Mask 0x0010 dwUnknown_v2
				cmReturn << DWORD(0x01);
				// Mask 0x00200000 - Burden
				cmReturn << WORD(0x0040);

				break;
			}
		case 1:
			{
				DWORD dwFlags2 = 0x00200038;

				cmReturn << dwFlags2 << pcModel->m_strName.c_str( ) << WORD(pcModel->m_wModel) << WORD(pcModel->m_wIcon);

				DWORD dwObjectFlags1 = 0x2000L;	
				cmReturn << dwObjectFlags1;

				DWORD dwObjectFlags2 = 0x0114L;
				cmReturn << dwObjectFlags2;
				
				// Masked against dwFlags2
				//Mask 0x0008 - Value
				cmReturn << DWORD(0x00);
				// Mask 0x0010 dwUnknown_v2
				cmReturn << DWORD(0x20);
				// Mask 0x0020 Approach distance
				cmReturn << float(3.0);
				// Mask 0x00200000 - Burden
				cmReturn << WORD(0x0040);

				break;
			}
		case 2:
			{
				DWORD dwFlags2 = 0x00200010;

				cmReturn << dwFlags2 << pcModel->m_strName.c_str( ) << WORD(pcModel->m_wModel) << WORD(pcModel->m_wIcon);

				DWORD dwObjectFlags1 = 0x2000L;	
				cmReturn << dwObjectFlags1;

				DWORD dwObjectFlags2 = 0x14L;
				cmReturn << dwObjectFlags2;
				
				// Masked against dwFlags2
				// Mask 0x0010 dwUnknown_v2
				cmReturn << DWORD(0x01);
				// Mask 0x00200000 - Burden
				cmReturn << WORD(0x0708);

				break;
			}
	}
	return cmReturn;
}	

void cWorldObject2::Action(cClient *who)
{
	switch(iObjectType)
	{
		case 1:
			{
				cMessage cmRead;
				cmRead << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x00B8L
					<< m_dwGUID << 0x01L << 0x01L << 0x03E8L << 0x01L  << 0xFFFFFFFF << "Welcome to Asheron's Call" << "beer good" << 0xFFFF0002
					<< 0L << 0L << 0L << 0L << 0L;
				//	<< 0x00L << 0x00L << m_strDescription.c_str();
				/*
				cMessage cmNotWorking;
				cmNotWorking << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x02EB
					<< "Reading this sign, not yet implemented! :p";
				*/
				who->AddPacket(WORLD_SERVER,cmRead,4);

				//Action Complete
				cMessage cmActionComplete;
				cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
				who->AddPacket(WORLD_SERVER,cmActionComplete,4);

				break;
			}
		default:
			{
				//Action Complete
				cMessage cmActionComplete;
				cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
				who->AddPacket(WORLD_SERVER,cmActionComplete,4);

				break;
			}
	}
}

void cWorldObject2::Assess(cClient *pcAssesser)
{
	//Action Complete
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}