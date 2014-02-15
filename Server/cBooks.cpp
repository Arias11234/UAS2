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
 *	@file cBooks.cpp
 *	Implements functionality for books.
 *
 *	This class is referenced whenever a book is created, used, or assessed.
 *	Inherits from the cObject class.
 */

#include "avatar.h"
#include "object.h"
#include "cItemModels.h"
#include "client.h"
#include "WorldManager.h"
#include "MasterServer.h"

/**
 *	Handles the message sent for the creation of books in the world.
 *
 *	This function is called whenever a book should be created in the world for a client.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cBooks::CreatePacket( )
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
	
	cmReturn << pcModel->m_dwFlags2;		// Object Flags
	cmReturn << Name( );					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << pcModel->m_wIcon;			// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags



	// Masked against dwFlags2
	if(pcModel->m_dwFlags2 & 0x00000008)
		cmReturn << pcModel->m_dwValue;
	
	// Mask 0x0010 dwUnknown_v2
	if(pcModel->m_dwFlags2 & 0x00000010)
		cmReturn << DWORD(0x00000008);
	
	// Mask 0x00200000 - Burden
	if(pcModel->m_dwFlags2 & 0x00200000)
		cmReturn << pcModel->m_wBurden;
	
	cmReturn << WORD(0x0000);


	return cmReturn;
}

/**
 *	Handles the message sent for the creation of books in a container.
 *
 *	This function is called whenever a book should be created in the inventory of another object.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cBooks::CreatePacketContainer(DWORD Container,DWORD ItemModelID)
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
	
	cmReturn << pcModel->m_dwFlags2;		// Object Flags

	cmReturn << Name( );					// Object's Name
	cmReturn << pcModel->m_wModel;			// Object's Model
	cmReturn << pcModel->m_wIcon;			// Object's Icon
	cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
	cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

		// Masked against dwFlags2
		if(pcModel->m_dwFlags2 & 0x00000008)
			cmReturn << pcModel->m_dwValue;
		
		// Mask 0x0010 dwUnknown_v2
		if(pcModel->m_dwFlags2 & 0x00000010)
			cmReturn << DWORD(0x00000008);

				// Mask 0x00000020
		//if(pcModel->m_dwFlags2 & 0x00000020)
		//	cmReturn << float(3.0);

		if(pcModel->m_dwFlags2 & 0x00004000)
			cmReturn << Container;
		
		// Mask 0x00200000 - Burden
		if(pcModel->m_dwFlags2 & 0x00200000)
			cmReturn << pcModel->m_wBurden;
		
		cmReturn << WORD(0x0000);

	return cmReturn;
}

/**
 *	Handles the actions of book objects.
 *
 *	This function is called whenever book is used or should perform an action.
 *	k109: This is the table of contents message.  Reading the pages function is below
 */
void cBooks::Action(cClient *who)
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	char password[100] = "beer good";
	cMessage cmRead;
	cmRead << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x00B4L
	<< m_dwGUID << pcModel->m_TotalPages << pcModel->m_ContentPages << 0x03E8L << pcModel->m_UsedPages;
	for(int i = 0; i < pcModel->m_TotalPages; i++)
	{
		cmRead << 0xFFFFFFFF << pcModel->m_Title.c_str() << pcModel->m_Author.c_str() << WORD(0x0002) << WORD(0xFFFF) << 0x0L << 0x0L;
	}

	/*
	<< m_dwGUID << pcModel->m_TotalPages << pcModel->m_ContentPages << 0x03E8L << pcModel->m_UsedPages << 0xFFFFFFFF << pcModel->m_Title.c_str() << pcModel->m_Author.c_str() << 0x00000000
	*/

	cmRead << pcModel->m_Comment.c_str() << who->m_pcAvatar->GetGUID() << pcModel->m_CommentAuthor.c_str();
	who->AddPacket(WORLD_SERVER,cmRead,4);

	//Action Complete
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);


}

void cBooks::Read(cClient *who, DWORD GUID, DWORD PageNumber)
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );
	char Password[100] = "Password is cheese";
	//char Booktext[512] = "Hail, traveler, welcome to Dereth! Your adventure awaits you. Your first task is to find the Greeter, who will train you in your new skills.\n\nTo enter the Training Academy, WALK INTO THE SPINNING PURPLE PORTAL.\n\nTo move, use the arrow keys to the left of the numeric keypad, or the keys surrounding the S key.  To use objects or talk to non-player characters (NPCs), double-click on them.";
	cMessage Read;
	Read << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x00B8L
	<< GUID << PageNumber << 0xFFFFFFFF << 0x0L << Password << WORD(0x0002) << WORD(0xFFFF) << 0x1L << 0x0L << pcModel->m_Pages[PageNumber].textPages.c_str();
	who->AddPacket(WORLD_SERVER,Read,4);
	
	cMasterServer::ServerMessage(ColorBlue,NULL,"PageNumber: %d",PageNumber);
	//Action Complete
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of book objects.
 *
 *	This function is called whenever a book is assessed by a client.
 *
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cBooks::Assess( cClient *pcAssesser )
{
	cItemModels *pcModel = cItemModels::FindModel( m_dwItemModelID );

	cMessage cmAssess;
	DWORD flags = 0x00000009;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L << m_dwGUID
	<< flags
	<< 0x01L						//Success = 0x01, Failure = 0x00
	<< WORD(0x0004)
	<< WORD(0x0010)
	<< 0x13L
	<< pcModel->m_dwValue
	<< 0x05L
	<< DWORD(pcModel->m_wBurden)
	<< 0xAEL
	<< pcModel->m_UsedPages
	<< 0xAFL
	<< pcModel->m_TotalPages
	<< WORD(0x0001)
	<< WORD(0x0008)
	<< 0x10L
	<< pcModel->m_strDescription.c_str();

	pcAssesser->AddPacket(WORLD_SERVER, cmAssess,4);

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}