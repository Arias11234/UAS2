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
 *	@file WarSpell.cpp
 */

#include "Object.h"
#include "WorldManager.h"
#include "Job.h"
#include "CorpseCleaner.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/**
 * Angle "val" is converted to (160 index), which corresponds to the ring of 16 sonar.
 * This index value is then returned.
 * Units are 1/10 degrees and 160 index (1/10th of 16).
 */
int deg_to_index_160(int val)
{
  int idx;
  idx = val * 2 / 45;         /* (idx = val/22.5) */
  if (idx < 0) idx += 160;
  return(idx);
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of war spells.
 *
 *	This function is called whenever a war spells should be created for a client.
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cWarSpell::CreatePacket()
{
	cMessage cmReturn;

	cMagicModels *pcModel = cMagicModels::FindModel( m_dwSpellModel );

	if( pcModel )
	{
		float	flpScale = 0.5f;  // Scale used in Packet being Sent

		cmReturn << 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant

		cmReturn << BYTE(0x0) << BYTE(0x0) << BYTE(0x0);

		DWORD dwFlags1 = pcModel->m_dwFlags1;// 0x0000FB85; // Flags 1 defines whats next
		cmReturn << dwFlags1;

		cmReturn << WORD(0x8B48);	// Type of portal 
		cmReturn << WORD(0x0002);	// Unknown
		
		
		//Flags1 Mask 0x0000FB85
		{
			//Flags1 & 0x00008000 Location 
			{
				//m_Location.m_flZ = m_Location.m_flZ + 1.4f;
				cmReturn.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );	//Next comes the location
			}
			//Flags1 & 0x00000800 
			{
				//SoundConfig
				cmReturn << 0x20000000 + pcModel->m_wSoundSet;//0x20000039;
			}
			//Flags1 & 0x00001000 
			{
				//Unknown_Blue   // Particle Effect for spell
				cmReturn << 0x34000000 + pcModel->m_wParticle; //0x3400000B; 
			}
			//Flags1 & 0x00000001
			{
				//DWORD dwModel = 0x02000001; //the model.
				cmReturn << 0x02000000 + pcModel->m_dwModelNumber;//0x020003F6;
			}
			//Flags1 & 0x00000080 unknown_green
			{
				//FLOAT flpScale
				cmReturn << pcModel->m_flScale;
			}
			//Flags1 & 0x00000100 
			{
				//Unkown_darkbrown
				cmReturn << 1.0f;//0x3F800000;
			}
			//Flags1 & 0x00000200 
			{
				//Unknown_BrightPurple
				cmReturn << 0x00000000;
			}
			//Flags1 & 0x00000004 
			{
				//Velocity Vector
				float flSpeed = 10;

				char Command2[100];
				sprintf(Command2,"dx: %f  dy: %f dz: %f Speed: %f\r\n", m_Velocity.m_dx, m_Velocity.m_dy, m_Velocity.m_dz, flSpeed);
				//UpdateConsole(Command2);

				cmReturn << (m_Velocity.m_dx*flSpeed);
				cmReturn << (m_Velocity.m_dy*flSpeed);
				cmReturn << (m_Velocity.m_dz*flSpeed);
			}
			//Flags1 & 0x00002000 
			{
				//
				cmReturn << 0x00000059;
			}
			//Flags1 & 0x00004000 
			{
				//
				cmReturn << 0x3F800000;
			}

		}
		// SeaGreens
		WORD wUnkFlag2 = 0x0000;
		WORD wUnkFlag3 = 0x0000;
		WORD wUnkFlag4 = 0x0001;
		WORD wUnkFlag6 = 0x0000;
		WORD wUnkFlag7 = 0x0000;
		WORD wUnkFlag8 = 0x0000;
		WORD wUnkFlag10 = 0x0000;

		cmReturn	<< m_wPositionSequence //movement 0x0001
					<< wUnkFlag2 // animations
					<< wUnkFlag3 // bubble modes
					<< wUnkFlag4 //num jumps
					<< m_wNumPortals 
					<< wUnkFlag6  //anim count
					<< wUnkFlag7  // overrides
					<< wUnkFlag8
					<< m_wNumLogins // 0x0D4B
					<< wUnkFlag10;
		
		DWORD dwFlags2 = 0x00400000;  //Flags2 Defines what data comes next
		cmReturn << dwFlags2;
		
		cmReturn << pcModel->m_strName;//Name( );	// Object's Name
		
		cmReturn << WORD(pcModel->m_wModel) << WORD(pcModel->m_wIcon); //m_wModelID << m_wIconID;

		DWORD dwObjectFlags1 = 0x00000000; // 0x0000 Nothing
		DWORD dwObjectFlags2 = 0x00000094; // 0x0004 Can not be picked up, 0x0080 Unknown - Can not be selected 0x0010 Unknown - Can be selected.
		// appears to contradict it self sets Can not be Selected and Can be Selected.  Housing Item, may set house open/closed

		cmReturn << dwObjectFlags1 << dwObjectFlags2;
		

		//Flags2 Mask
		{
		// 0x00400000 Word Associated Spell
			{
				WORD wUnknown9 = pcModel->m_wAssociatedSpell;//0x3A;
				cmReturn << wUnknown9;
			}
		}
		//cmReturn << WORD(0x0000);
	}
	else
	{
		UpdateConsole(" Spell Error! %d\r\n", m_dwSpellModel);
	}
  return cmReturn;
}

/**
 *	Handles the movement of war spells.
 *
 *	This function is called whenever a war spell's position should be updated.
 */
int cWarSpell::Move( LPVOID wp, LPVOID lp )
{
	cSpellMoveParam* SpellMoveParam = (cSpellMoveParam *) wp;
	cWarSpell *pcWarSpell = (cWarSpell*) cWorldManager::FindObject( SpellMoveParam->GetWarSpellGUID() );
	cClient *pcClient = cClient::FindClient( SpellMoveParam->GetCasterGUID() );
	
	cLocation newLoc = cPhysics::VelocityMove(pcWarSpell->m_Location, pcWarSpell->m_Velocity, 1.0f);

	if ( HIWORD(newLoc.m_dwLandBlock) != HIWORD(pcWarSpell->m_Location.m_dwLandBlock) )
	{
		cWorldManager::MoveRemObject( pcWarSpell );
		pcWarSpell->m_Location = newLoc;
		cWorldManager::MoveAddObject( pcWarSpell );
	}
	else
		pcWarSpell->m_Location = newLoc;

	char Command[100];
	sprintf(Command,"Move! X: %f  Y: %f\r\n", pcWarSpell->m_Location.m_flX, pcWarSpell->m_Location.m_flY);
	//UpdateConsole(Command);
	if ( cMonster *CollisionMonster = (cMonster*) cWorldManager::GetCollisionMonster( pcWarSpell->GetGUID(), pcWarSpell->m_Location ) )
	{
		//UpdateConsole(" Hit!\r\n");

		cMessage msgImpact = pcWarSpell->SpellImpact(pcWarSpell,0x0005,1.0f);
		cWorldManager::SendToAllInFocus( pcWarSpell->m_Location, msgImpact, 3 );

		cMessage msgVis = pcWarSpell->SpellVis(pcWarSpell);
		cWorldManager::SendToAllInFocus( pcWarSpell->m_Location, msgVis, 3 );

		CollisionMonster->SpellAttack( pcClient , pcWarSpell, ++pcClient->m_dwF7B0Sequence);
		SimpleAI::SetUnderAttack( CollisionMonster->GetGUID(), pcClient->m_pcAvatar->GetGUID() );

		CorpseCleaner::AddCorpse(pcWarSpell->GetGUID(),2);
		SAFEDELETE( SpellMoveParam )
		return 2;
	}
	else if ( pcWarSpell->m_Location.m_flZ < cPhysics::GetLandZ( pcWarSpell->m_Location ) )
	{
		//UpdateConsole(" Land hit!\r\n");

		cMessage msgImpact = pcWarSpell->SpellImpact(pcWarSpell,0x0005,1.0f);
		cWorldManager::SendToAllInFocus( pcWarSpell->m_Location, msgImpact, 3 );

		cMessage msgVis = pcWarSpell->SpellVis(pcWarSpell);
		cWorldManager::SendToAllInFocus( pcWarSpell->m_Location, msgVis, 3 );

		CorpseCleaner::AddCorpse(pcWarSpell->GetGUID(),2);
		SAFEDELETE( SpellMoveParam )
		return 2;
	}

	if ( cPhysics::Get3DRange( SpellMoveParam->GetCastLocation(), pcWarSpell->m_Location ) > 3.3333 )
	{
		UpdateConsole(" Dissipate.\r\n");
		CorpseCleaner::AddCorpse(pcWarSpell->GetGUID(),2);
		SAFEDELETE( SpellMoveParam )
		return 2;
	}

	cMessage msgSpellPosition = pcWarSpell->SetPosition();
	//cWorldManager::SendToAllInFocus( pcWarSpell->m_Location, msgSpellPosition, 3 );
	return -1;
}

/**
 *	Handles the general actions of war spell objects.
 */
void cWarSpell::Action(cClient *who)
{
	
}

/**
 *	Handles the assessment of war spell objects.  This should not be called.
 */
void cWarSpell::Assess(cClient *pcAssesser)
{

}

/**
 *	Particle effects for war spell objects.
 *
 *	This function is called when a war spell should display a particle effect.
 *	@return cMessage - Returns an Apply Visual/Sound Effect (0x0000F755) server message.
 */
cMessage cWarSpell::WarParticle(cWarSpell* pcWarSpell,DWORD dwEffect,float flSpeed)
{
	cMessage cmWarParticle;
	static BYTE cbApplyEffect[] = {
		0x55, 0xF7, 0x00, 0x00,
		0xBA, 0x2E, 0xC5, 0xC2,
		0x04, 0x00, 0x00, 0x00,
		0x3F, 0x80, 0x00, 0x00,
	};

	CopyMemory( &cbApplyEffect[4], &pcWarSpell->m_dwGUID, 4 );
	CopyMemory( &cbApplyEffect[8], &dwEffect, 4 );
	CopyMemory( &cbApplyEffect[12], &flSpeed, 4 );

	
	cmWarParticle.CannedData( cbApplyEffect, sizeof( cbApplyEffect ) );

	return cmWarParticle;
}

/**
 *	Impact effects for war spell objects.
 *
 *	This function is called when a war spell should display an impact effect.
 *	@return cMessage - Returns an Apply Visual/Sound Effect (0x0000F755) server message.
 */
cMessage	cWarSpell::SpellImpact(cWarSpell* pcWarSpell, DWORD dwEffect, float flSpeed)
{
	cMessage cmSpellImpact;
	static BYTE cbApplyImpact[] = {
		0x55, 0xF7, 0x00, 0x00,
		0xBA, 0x2E, 0xC5, 0xC2,
		0x04, 0x00, 0x00, 0x00,
		0x3F, 0x80, 0x00, 0x00,
	};

	CopyMemory( &cbApplyImpact[4], &pcWarSpell->m_dwGUID, 4 );
	CopyMemory( &cbApplyImpact[8], &dwEffect, 4 );
	CopyMemory( &cbApplyImpact[12], &flSpeed, 4 );

	cmSpellImpact.CannedData( cbApplyImpact, sizeof( cbApplyImpact ) );

	return cmSpellImpact;
}

/**
 *	Toggles war spell visibility.
 *
 *	This function is called when the visibility of a war spell should change for a client.
 *	@return cMessage - Returns a Toggle Object Visibility (0x0000F74B) server message.
 */
cMessage	cWarSpell::SpellVis(cWarSpell* pcWarSpell)
{
	cMessage cmSpellVis;
	static BYTE cbApplyVis[] = {
		0x4B, 0xF7, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x74, 0x83, //Portal type
		0x12, 0x00, //Unknown word
		0x06, 0x00, //NumLogins
		0x01, 0x00, //NumPortals
		0x43, 0x3D,
		0x09, 0x01,
	};

	CopyMemory( &cbApplyVis[4], &pcWarSpell->m_dwGUID, 4 );

	cmSpellVis.CannedData( cbApplyVis, sizeof( cbApplyVis ) );

	return cmSpellVis;
}

/**
 *	Sends a war spell animation.
 *
 *	This function is called when an animation for a war spell should sent to a client.
 *	@return cMessage - Returns a Set Animation (0x0000F74C) server message.
 */
cMessage	cWarSpell::SpellAnim(cWarSpell* pcWarSpell,WORD wWarAnim,WORD wWarAnim2)
{
	cMessage cmSpellAnim;
	static BYTE cbSpellAnim[] = {
		0x4C, 0xF7, 0x00, 0x00, 
		0xBA, 0x31, 0xC5, 0xC2, //GUID
		0x01, 0x00, //numLogins
		0x03, 0x00, //sequence
		0x03, 0x00, //numAnim
		0x00, 0x00, //activity
		0x00, 0x00, //animation Type
		0x3C, 0x00, //Stance
		0x01, 0xF0, 0x99, 0x02, //flags 
		0x3C, 0x00, 0x00, 0x00, //stanceMode2
		0x3F, 0x80, 0x00, 0x00,
	};

	CopyMemory( &cbSpellAnim[4], &pcWarSpell->m_dwGUID, 4 );
	CopyMemory( &cbSpellAnim[18], &wWarAnim, 2 );
	CopyMemory( &cbSpellAnim[24], &wWarAnim2, 2 );

	
	cmSpellAnim.pasteData( cbSpellAnim, sizeof( cbSpellAnim ) );

	return cmSpellAnim;
}

/**
 *	Sets the location for war spell objects.
 *
 *	This function is called whenever the location of a war spell should be updated for a client.
 *	@return cMessage - Returns a Set Position and Motion (0x0000F748) server message.
 */
cMessage cWarSpell::SetPosition()
{
	cMessage cmSetPosition;
	DWORD dwFlags = 0x01L;

	cmSetPosition	<< 0xF748L
					<< m_dwGUID	
					<< 0x01L;

	cmSetPosition	<< m_Location.m_dwLandBlock
					<< m_Location.m_flX
					<< m_Location.m_flY
					<< m_Location.m_flZ
					<< m_Location.m_flA
					<< m_Location.m_flB
					<< m_Location.m_flC
					<< m_Location.m_flW
					<< m_Velocity.m_dx
					<< m_Velocity.m_dy
					<< m_Velocity.m_dz;

	return cmSetPosition;
}