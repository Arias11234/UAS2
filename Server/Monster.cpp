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
 *	@file Monster.cpp
 *	Implements functionality for monsters.
 *
 *	Includes member functions to control the movement, attacking, respawning, animation, and assessment of monsters.
 */

#include "Client.h"
#include "MasterServer.h"
#include "Object.h"
#include "WorldManager.h"
#include "CorpseCleaner.h"
#include "Database.h"
#include "Job.h"
#include "cSpell.h"

/***************
 *	constructors
 **************/

/**
 *	Handles the creation of monsters.
 *
 *	Called whenever a monster object should be initialized.
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMonster::cMonster( DWORD dwGUID, DWORD dwModelID, cLocation *pcLoc, char *szName, char *szDesc, cMonStats *pcmsStats, DWORD dwRespawn, DWORD dwDecay, DWORD dwChase, DWORD dwInfluence, DWORD dwExp_Value, DWORD dwHealth, DWORD dwStamina, DWORD dwMana )
	:	m_dwMonsterModelID( dwModelID )
{
	SetLocation( pcLoc );
	SetSpawnLoc( pcLoc );
	m_dwGUID				=	dwGUID;
	m_dwMonsterModelID		=	dwModelID;
	m_fDeadOrAlive			=	true; // true = Alive, false = Dead
	m_strName.assign( szName );
	m_strDescription.assign( szDesc );
	m_wNumMovements			= 0x0000;
	m_wNumAnimInteracts		= 0x0000;
	m_wNumBubbleModes		= 0x0000;
	m_wNumJumps				= 0x0000;
	m_wNumPortals			= 0x0000;
	m_wAnimCount			= 0x0000;
	m_wNumOverrides			= 0x0000;
	m_wNumLogins			= 0x0001;
	m_fStatic				= TRUE;
	m_dwObjectFlags1		= 0x0L;
	m_dwObjectFlags2		= 0x0L;
	m_wNumAnims			= 0x0000;
	m_wCurAnim			= 0x0000;
	m_wMeleeSequence	= 0x0000;
	m_dwF7B0Sequence	= 0x00000000;
	m_bStatSequence		= 0x00;
	m_fCombatMode		= false;
	m_dwReSpawn			= dwRespawn;
	m_dwDecay			= dwDecay;
	m_dwChase			= dwChase;
	m_dwInfluence		= dwInfluence;
	m_dwExp_Value		= dwExp_Value;
	m_iPosUpdateCount	= 10;
	
	// Set the Character Stats to 0 for safety

	//Cubem0j0:  Modified this, monster vitals are not always the same as pc's.
	m_dwLevel			= pcmsStats->m_dwLevel;
	m_dwMaxHealth		= dwHealth;
	m_dwStr				= pcmsStats->m_dwStr;
	m_dwEnd				= pcmsStats->m_dwEnd;
	m_dwQuick			= pcmsStats->m_dwQuick;
	m_dwCoord			= pcmsStats->m_dwCoord;
	m_dwFocus			= pcmsStats->m_dwFocus;
	m_dwSelf			= pcmsStats->m_dwSelf;
	m_dwMaxStamina		= dwStamina;
	m_dwMaxMana			= dwMana;
	m_dwSpecies			= pcmsStats->m_dwSpecies;
	m_dwCurrenthealth	= m_dwMaxHealth;
	m_dwCurrentStamina	= m_dwMaxStamina;
	m_dwCurrentMana		= m_dwMaxMana;
	m_bIdleAnim			= 0;
	m_bCombatMode		= 0;
	
	static BYTE bAnimate[8] = {
		0x00, 0x00, 0x3D, 0x00, 0x00, 0xF0, 0xB5, 0x02,
	};

	cModels *pcModel = cModels::FindModel( m_dwMonsterModelID );
	if( pcModel )
	{
		m_dwUnknownCount = pcModel->m_dwUnknownCount;
		CopyMemory( &m_bInitialAnimation[0], pcModel->m_bInitialAnimation, pcModel->m_dwUnknownCount);
	}
	else
	{
		m_dwUnknownCount = sizeof(bAnimate);
		CopyMemory( &m_bInitialAnimation[0], &bAnimate, 8);
	}
	
	SimpleAI::AddMonster( m_dwGUID );
}

/**********
 *	methods
 *********/

/**
 *	Handles the message sent for the creation of monsters.
 *
 *	This member function is called whenever a monster should be created for a client.
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cMonster::CreatePacket()
{
	cMessage cmReturn;
	cModels *pcModel = cModels::FindModel( m_dwMonsterModelID );

	DWORD dwFlags1;

	if( pcModel )
	{
		DWORD dwUnkDatEntry2 = 0x0L;
		m_dwObjectFlags1 = pcModel->m_dwObjectFlags1;
		m_dwObjectFlags2 = pcModel->m_dwObjectFlags2;

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
		// End Model Vectors
		dwFlags1 = pcModel->m_dwFlags1;
		if( pcModel->m_dwFlags1 & 0x40)
		{
			dwFlags1 = dwFlags1 - 0x40;
		}
		if( pcModel->m_dwFlags1 & 0x20)
		{
			dwFlags1 = dwFlags1 - 0x20;
		}
		cmReturn.pasteAlign(4);

		cmReturn	<< dwFlags1;
		cmReturn    << WORD(pcModel->m_wPortalMode)

			<< WORD(pcModel->m_wUnknown_1);

		// Mask against dwFlags1
		// Choose Valid Sections Masking Against Flags1
		//Flags1 &  0x00010000
		if (pcModel->m_dwFlags1 &  0x00010000 )
		{
			cmReturn << m_dwUnknownCount;			//pcModel->m_dwUnknownCount;
			// Vector of UnknownCount
			for ( int i = 0;i < m_dwUnknownCount; i++)
			{
				//pcModel->m_dwUnknownCount; i++) {
				cmReturn << m_bInitialAnimation[i]; //pcModel->m_bInitialAnimation[i]; 
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
			cmReturn.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );
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
		// Flags1 & 0x00040000 - Unknown
		if (pcModel->m_dwFlags1 & 0x00040000)
		{
			//Format = 0x02000001; 
			cmReturn << 0x02000000 + pcModel->m_dwUnknown_LightGrey;
		}
		// Unknown Trios
		// Flags1 & 0x00000004
		if (pcModel->m_dwFlags1 & 0x00000004)
		{
			cmReturn << pcModel->m_dwTrio1[0] << pcModel->m_dwTrio1[1] << pcModel->m_dwTrio1[2];
		}
		// Flags1 & 0x00000008
		if (pcModel->m_dwFlags1 & 0x00000008)
		{
			cmReturn << pcModel->m_dwTrio2[0] << pcModel->m_dwTrio2[1] << pcModel->m_dwTrio2[2];
		}
		// Flags1 & 0x00000010
		if (pcModel->m_dwFlags1 & 0x00000010)
		{
			cmReturn << pcModel->m_dwTrio3[0] << pcModel->m_dwTrio3[1] << pcModel->m_dwTrio3[2];
		}
			// Unknown Values 
		// Flags1 & 0x00002000
		if (pcModel->m_dwFlags1 & 0x00002000)
		{
			cmReturn << pcModel->m_dwMedGrey;
		}
		// Flags1 & 0x00004000
		if (pcModel->m_dwFlags1 & 0x00004000)
		{
			cmReturn << pcModel->m_dwBlueGrey;
		}

		// This set of Data is found in every Create Object Packet
		cmReturn << m_wNumMovements;
		cmReturn << m_wNumAnimInteracts;
		cmReturn << m_wNumBubbleModes;
		cmReturn << m_wNumJumps;
		cmReturn << m_wNumPortals;
		cmReturn << m_wAnimCount;
		cmReturn << m_wNumOverrides;
		cmReturn << pcModel->m_wSeagreen8;
		cmReturn << m_wNumLogins;
		cmReturn << pcModel->m_wSeagreen10;

		cmReturn << pcModel->m_dwFlags2;		// Object Flags 
		cmReturn << Name( );					// Objects Name
		cmReturn << pcModel->m_wModel;			// Objects Model
		cmReturn << pcModel->m_wIcon;			// Objects Icon
		cmReturn << pcModel->m_dwObjectFlags1;	// Object Flags
		cmReturn << pcModel->m_dwObjectFlags2;	// Object Flags

		pcModel->mob_strength = m_dwStr;

		//Cubem0j0: Approach distance used in combat routine
		this->m_fApproachDistance = 0.3f;

		// Choose Valid Sections Masking Against Flags2
		// Flags2 & 0x00000002 - Item Slots
		if (pcModel->m_dwFlags2 & 0x00000002)
		{
			cmReturn << pcModel->m_bItemSlots;
		}
		// Flags2 & 0x00000004 - Pack Slots
		if (pcModel->m_dwFlags2 & 0x00000004)
		{
			cmReturn << pcModel->m_bPackSlots;
		}
		// Flags2 & 0x04000000 - Behavior
		//if (pcModel->m_dwFlags2 & 0x04000000)
		//{
		//	cmReturn << pcModel->m_bBehavior;
		//}
		// Flags2 & 0x00000010
		if (pcModel->m_dwFlags2 & 0x00000010)
		{
			cmReturn << pcModel->m_dwUnknown_v2;
		}
		// Flags2 & 0x00800000
		if (pcModel->m_dwFlags2 & 0x00800000)
		{
			cmReturn << pcModel->m_dwUnknown_v6;
		}
		return cmReturn;
	}
	else
	{
		cmReturn << 0xF755L << DWORD(1) << DWORD(124) << 0x3F2B4E94L;
		return cmReturn;
	}
}

/**
 *	Handles the respawning of monster objects .
 *
 *	This member function is called whenever a monster should be respawned.
 *
 *	@param *pcObject - A pointer to the monster object to be respawned.
 */
void cMonster::ReSpawn( cObject *pcObject )
{
	m_fDeadOrAlive		= true; 
	m_fCombatMode		= false;
	m_dwCurrenthealth	= m_dwMaxHealth;
	
	static BYTE bAnimate[8] = {
		0x00, 0x00, 0x3D, 0x00, 0x00, 0xF0, 0xB5, 0x02,
	};
	m_dwUnknownCount = sizeof(bAnimate);
	CopyMemory( &m_bInitialAnimation[0], &bAnimate, 8);

	cWorldManager::MoveRemObject( this );
	SetLocation( m_SpawnLoc );
	cWorldManager::AddObject( this, true );
	SimpleAI::AddMonster( m_dwGUID );
}

void cMonster::MonsterCorpse( )
{
}

cMessage cMonster::Animation( WORD wAnim, float flPlaySpeed)
{
	// Animation Group 3 Event type
	cMessage cAnim;

	static BYTE bAnimate[] = {
		0x4C, 0xF7, 0x00, 0x00, // Type 
		0x3F, 0x41, 0xD1, 0x0B, // dwGUID
		0x00, 0x00,				// numLogins 
		0x01, 0x00,				// Sequence
		0x01, 0x00,				// numAnims
		0x00, 0x00,				// activity 0x0 - idle 0x1 - active
		0x00,					// Animation Type
		0x00,					// type Flags
		0x3D, 0x00,				// Stance Mode 0x3D standing
		0x43, 0xE0, 0xB5, 0x02, // flags
		0x3D, 0x00,				// Stance Mode 2
		0x00, 0x00,				// Animation ID 
		0x00, 0x00, 0xC0, 0x3F, // flPlaySpeed (1.5)
		0x79, 0xFA, 0x29, 0x01,
	};

	++m_wCurAnim;
	++m_wMeleeSequence;
	CopyMemory( &bAnimate[04],   &m_dwGUID, 4 );
	CopyMemory( &bAnimate[8],    &m_wNumLogins, 2 );
	CopyMemory( &bAnimate[10],   &m_wCurAnim, 2 );
	CopyMemory( &bAnimate[12],   &m_wMeleeSequence, 2 );
	CopyMemory( &bAnimate[26],	 &wAnim, 2 );
	CopyMemory( &bAnimate[28],   &flPlaySpeed, 4 );

	cAnim.CannedData( bAnimate, sizeof( bAnimate ) );
	return cAnim;
}

/**
 *	Handles functionality for when monster objects are physically attacked.
 *
 *	This member function is called whenever a monster is physically attacked.
 *
 *	@param *who - A pointer to the client physically attacking the monster.
 *	@param flDamageSlider - The value of the client's damage slider.
 *	@Param F7B0 Sequence - The given client's F7B0 sequence number.
 */
void cMonster::Attack(cClient* who, float flDamageSlider, DWORD F7B0Sequence)
{
	// This is for Combat Routines 
	DWORD dwDamageType = 0x04;

	/*
	if (who->m_pcAvatar->myWeapon->m_fEquipped == 1)
		dwDamageType = who->m_pcAvatar->myWeapon->m_dwDamageType;
	*/

	//cMessage Anim = who->m_pcAvatar->CombatAnimation( GetGUID() );
	cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, who->m_pcAvatar->CombatAnimation( GetGUID() ), 3 );

	WORD wTotalOpponentHealth = GetTotalHealth();

	DWORD dwDamage = who->m_pcAvatar->CalculateDamage( NULL, flDamageSlider, 0.0f );
	double dSeverity = ( double ) wTotalOpponentHealth / dwDamage;

	int iNewHealth;

	BOOL fKilledPlayer = FALSE;
	cMessage cmHealthLoss;

	m_dwCurrenthealth -= dwDamage;
	iNewHealth = m_dwCurrenthealth;
	
	double flHealthStat = ((( double )100/ m_dwMaxHealth)* m_dwCurrenthealth)/100;

	cmHealthLoss << 0xF7B0L 
				 << who->m_pcAvatar->GetGUID() 
				 << F7B0Sequence
				 << 0x01C0L 
				 << m_dwGUID 
				 << float(flHealthStat);

	who->AddPacket( WORLD_SERVER, cmHealthLoss, 4 );

	if( iNewHealth <= 0 ) 
	{
		cMessage cmHealthLoss;

		//set health to 0
		m_dwCurrenthealth = 0;

		cmHealthLoss << 0xF7B0L 
				 << who->m_pcAvatar->GetGUID() 
				 << F7B0Sequence
				 << 0x01C0L 
				 << m_dwGUID 
				 << float(flHealthStat);

		who->AddPacket( WORLD_SERVER, cmHealthLoss, 4 );

		//m_fDeadOrAlive = false;

		char szDeathNotice[200];
		wsprintf( szDeathNotice, "%s has killed %s", who->m_pcAvatar->m_strName.c_str(), m_strName.c_str( ) );
		
		//This line actually makes the monster not continue to fight:
		SimpleAI::RemoveMonster( m_dwGUID );

		//Death notice and animation
		cMessage cmDeathNotice;
		cmDeathNotice << 0xF62C << szDeathNotice << ColorGreen;
		cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cmDeathNotice, 4 );
		cMessage cMonAnim = Animation( 0x11L, 1.5f );
		cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cMonAnim, 3 );

		static BYTE bDead[12] = {
			0x00, 0x00, 0x3D, 0x00, 0x02, 0xF0, 0xB5, 0x02, 0x11, 0x00, 0x00, 0x00,
		};

		m_dwUnknownCount = sizeof(bDead);
		CopyMemory( &m_bInitialAnimation[0], &bDead, 12);
		//cMasterServer::m_pcCorpse->AddCorpse(m_dwGUID,m_dwDecay,m_dwReSpawn);
		m_fDeadOrAlive = false;

		if(m_fDeadOrAlive == false)
		{
			//cMessage cmUpdateXP = who->m_pcAvatar->UpdateUnassignedExp(m_dwExp_Value);
			who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdateUnassignedExp(m_dwExp_Value), 4 );
			
			who->m_pcAvatar->UpdateUnassignedXPDB(who,m_dwExp_Value);

			//Update DB Unassigned experience
			//char			szCommand[100];
			//RETCODE			retcode;
			//sprintf( szCommand, "UPDATE avatar set UnassignedXP = UnassignedXP + %i WHERE AvatarGUID = %lu;",m_dwExp_Value,who->m_pcAvatar->GetGUID( ));
			//retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			//retcode = SQLExecute( cDatabase::m_hStmt );
			//retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			//retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			

			//cMessage cmUpdateXP2 = who->m_pcAvatar->UpdateTotalExp(m_dwExp_Value);
			who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdateTotalExp(m_dwExp_Value), 4);		
			who->m_pcAvatar->UpdateTotalXPDB(who,m_dwExp_Value);

			//Update DB Total experience
			//char			szCommand2[100];
			//RETCODE			retcode2;
			//sprintf( szCommand2, "UPDATE avatar set TotalXP = TotalXP + %i WHERE AvatarGUID = %lu;",m_dwExp_Value,who->m_pcAvatar->GetGUID( ));
			//retcode2 = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand2, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			//retcode2 = SQLExecute( cDatabase::m_hStmt );
			//retcode2 = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			//retcode2 = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			//check to see if we gained a level

			//cMessage cmUpdateLevel = who->m_pcAvatar->UpdateLevel();
			who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdateLevel(), 4);
			who->m_pcAvatar->UpdateLevelDB(who);

			//Update level in DB
			//char			szCommand3[100];
		//	RETCODE			retcode3;
		//	sprintf( szCommand3, "UPDATE avatar set Level = %i WHERE AvatarGUID = %lu;",who->m_pcAvatar->m_cStats.m_dwLevel,who->m_pcAvatar->GetGUID( ));
		//	retcode3 = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand3, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		//	retcode3 = SQLExecute( cDatabase::m_hStmt );
		//	retcode3 = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		//	retcode3 = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			//Test to update Pyreals...seems to work.

			//cMessage GivePy = who->m_pcAvatar->UpdatePyreals(100,1);
			who->AddPacket( WORLD_SERVER, who->m_pcAvatar->UpdatePyreals(100,1), 4);
				
			cObject* pcMonster = cWorldManager::FindObject( m_dwGUID );
			cModels* pcModel = cModels::FindModel( m_dwMonsterModelID );
			if(pcModel)
			{
				char CorpseName[100];
				char KilledBy[255];
				//Name of the corpse
				sprintf(CorpseName,"Corpse of %s",pcMonster->m_strName.c_str( ));
				//Who killed it (its description)
				sprintf(KilledBy,"Killed by %s",who->m_pcAvatar->m_strName.c_str());

				cCorpse* moncor = new cCorpse(pcModel->m_dwModelNumber,cWorldManager::NewGUID_Object(),pcMonster->m_Location,CorpseName,KilledBy);
				moncor->SetData(pcModel->m_wAnimConfig,pcModel->m_wSoundSet,pcModel->m_wModel,WORD(0x1070),m_dwMonsterModelID,pcModel->m_flScale);

				//Add a copy of the monster as a corpse.
				cWorldManager::AddObject(moncor,true);

				//Make it disappear after 120 seconds.
				CorpseCleaner::AddCorpse(moncor->GetGUID(),120,0);

				//Remove the real monster
				cMessage cmRemModel;
				cmRemModel << 0xF747L << m_dwGUID << DWORD( 1 );
				cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, cmRemModel, 3 );
				SimpleAI::RemoveMonster(m_dwGUID);
				cWorldManager::RemoveObject( pcMonster );
			}
		}

		SimpleAI::SetAttackComplete( m_dwGUID );
	}
	else
	{
		//cMessage cmDamageDoneMessage = who->m_pcAvatar->DoDamageMessage( ++who->m_dwF7B0Sequence, m_strName, dwDamageType, dSeverity, dwDamage);
		who->AddPacket( WORLD_SERVER, who->m_pcAvatar->DoDamageMessage( ++who->m_dwF7B0Sequence, m_strName, dwDamageType, dSeverity, dwDamage), 4 );	

		//Cubem0j0:  Reduce stamina for combat, this is a test  NEED TO FINISH THIS :)
		int iShieldBurden = 200; //Will eventually be Shields actual burden units.
		int iWeaponBurden = 100; //Will eventually be weapons actual burden units.
		
		int iTrueBurden = (iWeaponBurden * 1.5  ) + iShieldBurden;
		int iNewStamina = who->m_pcAvatar->GetTotalStamina();

		switch(iTrueBurden)
		{
		case 0-220:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(2,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
		
		case 221-998:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(3,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
		case 999-1665:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(4,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
		case 1666-2355:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(5,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
		case 2356-3023:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(6,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
		case 3024-3450:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(7,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;			
		case 3451-4110:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(8,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
			
		default:
			{
				cMessage cmReduceStamina = who->m_pcAvatar->DecrementStamina(1,iNewStamina);
				who->AddPacket(WORLD_SERVER,cmReduceStamina,4);
			}
			break;
		}
	}
}

/**
 *	Handles functionality for when spells are cast on monster objects.
 *
 *	This member function is called whenever a spell is cast on a monster.
 *
 *	@param *who - A pointer to the client who cast the spell.
 *	@param *pcWarSpell - A pointer to the war spell cast by the client.
 *	@param F7B0Sequence - The given client's F7B0 sequence number.
 */
void cMonster::SpellAttack(cClient* who, cWarSpell* pcWarSpell, DWORD F7B0Sequence)
{
	// This is for Spell Routines
	std::string		strDamage1, strDamage2;
	cMagicModels *pcModel = cMagicModels::FindModel( pcWarSpell->m_dwSpellModel );
	cSpell *pcSpell = cSpell::FindSpell( pcWarSpell->m_dwSpellID );
	
	if (pcModel)
	{
		switch(pcModel->m_dwModelNumber)
		{
		case 0x03FC:
			{
				strDamage1 = "slice";
				strDamage2 = "slices";
				break;
			}
		case 0x03F3:
			{
				strDamage1 = "pierce";
				strDamage2 = "pierces";
				break;
			}
		case 0x03FA:
			{
				strDamage1 = "crush";
				strDamage2 = "crushes";
				break;
			}
		case 0x03F4:
			{
				strDamage1 = "freeze";
				strDamage2 = "freezes";
				break;
			}
		case 0x040D:
			{
				strDamage1 = "burn";
				strDamage2 = "burns";
				break;
			}
		case 0x03F6:
			{
				strDamage1 = "sear";
				strDamage2 = "sears";
				break;
			}
		case 0x03F0:
			{
				strDamage1 = "shock";
				strDamage2 = "shocks";
				break;
			}
		default:
			{
				strDamage1 = "hurt";
				strDamage2 = "hurts";
				break;
			}
		}
	}
	WORD wTotalOpponentHealth = GetTotalHealth();

	srand( timeGetTime( ) );
	int minDam = 61;
	int maxDam = 120;
	if (pcSpell)
	{
		switch(pcSpell->m_dwLevel)
		{
		case 1:
			{
				minDam = 8;
				maxDam = 15;
				break;
			}
		case 2:
			{
				minDam = 13;
				maxDam = 25;
				break;
			}
		case 3:
			{
				minDam = 18;
				maxDam = 35;
				break;
			}
		case 4:
			{
				minDam = 31;
				maxDam = 60;
				break;
			}
		case 5:
			{
				minDam = 46;
				maxDam = 90;
				break;
			}
		case 6:
			{
				minDam = 61;
				maxDam = 120;
				break;
			}
		case 7:
			{
				minDam = 110;
				maxDam = 180;
				break;
			}
		}
	}
	DWORD dwDamage = rand() % (maxDam-minDam) + minDam;
	double dSeverity = ( double ) wTotalOpponentHealth / dwDamage;

	int iNewHealth;

	BOOL fKilledPlayer = FALSE;
	cMessage cmHealthLoss;

	m_dwCurrenthealth -= dwDamage;

	if (m_dwCurrenthealth < 0)
		m_dwCurrenthealth = 0;

	iNewHealth = m_dwCurrenthealth;

	double flHealthStat = ((( double )100/ m_dwMaxHealth)* m_dwCurrenthealth)/100;

	cmHealthLoss << 0xF7B0L 
				 << who->m_pcAvatar->GetGUID() 
				 << F7B0Sequence
				 << 0x01C0L 
				 << m_dwGUID 
				 << float(flHealthStat);

	who->AddPacket( WORLD_SERVER, cmHealthLoss, 4 );
	
	char szDamage1[200];
	char szDamage2[200];
	wsprintf( szDamage1, "You %s %s for %d points of damage with %s!", strDamage1.c_str(), m_strName.c_str(), dwDamage, pcSpell->m_strName.c_str() );
	wsprintf( szDamage2, "%s %s %s for %d points of damage!", who->m_pcAvatar->m_strName.c_str(), strDamage1.c_str(), m_strName.c_str(), dwDamage );
	cMessage cmDamage1;
	cMessage cmDamage2;
	cmDamage1 << 0xF62C << szDamage1 << ColorPink;
	cmDamage2 << 0xF62C << szDamage2 << ColorPink;
	who->AddPacket( WORLD_SERVER, cmDamage1, 4 );
	cWorldManager::SendToOthersInFocus( who->m_pcAvatar->m_Location, who, cmDamage2, 4 );
	
	//if the monster is killed
	if( iNewHealth <= 0 ) 
	{
		//This line actually makes the monster discontinue fighting
		SimpleAI::RemoveMonster( m_dwGUID );

		//Death notice and animation
		char szDeathNotice[200];
		char szDeathNotice2[200];
		wsprintf( szDeathNotice, "%s has killed %s!", who->m_pcAvatar->m_strName.c_str(), m_strName.c_str( ) );
		wsprintf( szDeathNotice2, "You have killed %s!", m_strName.c_str( ) );
		cMessage cmDeathNotice;
		cMessage cmDeathNotice2;
		cmDeathNotice << 0xF62C << szDeathNotice << ColorGreen;
		cmDeathNotice2 << 0xF62C << szDeathNotice2 << ColorGreen;
		cWorldManager::SendToOthersInFocus( who->m_pcAvatar->m_Location, who, cmDeathNotice, 4 );
		who->AddPacket( WORLD_SERVER, cmDeathNotice2, 4 );
		cMessage cMonAnim = Animation( 0x11L, 1.5f );
		cWorldManager::SendToAllInFocus( who->m_pcAvatar->m_Location, cMonAnim, 3 );

		static BYTE bDead[12] = {
			0x00, 0x00, 0x3D, 0x00, 0x02, 0xF0, 0xB5, 0x02, 0x11, 0x00, 0x00, 0x00,
		};

		m_dwUnknownCount = sizeof(bDead);
		CopyMemory( &m_bInitialAnimation[0], &bDead, 12);
		//cMasterServer::m_pcCorpse->AddCorpse(m_dwGUID,m_dwDecay,m_dwReSpawn);
		m_fDeadOrAlive = false;

		if(m_fDeadOrAlive == false)
		{
			who->m_pcAvatar->UpdateHuntingExp(m_dwExp_Value);

			//Update DB Unassigned experience
			char		szCommand[512];
			RETCODE		retcode;
			sprintf( szCommand, "UPDATE avatar set UnassignedXP = UnassignedXP + %i WHERE AvatarGUID = %lu;",m_dwExp_Value,who->m_pcAvatar->GetGUID( ));
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLExecute( cDatabase::m_hStmt );
			retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	
			cMessage cmUpdateXP2 = who->m_pcAvatar->UpdateTotalExp(m_dwExp_Value);
			who->AddPacket( WORLD_SERVER, cmUpdateXP2, 4);
	
			//Update DB Total experience
			char		szCommand2[512];
			RETCODE		retcode2;
			sprintf( szCommand2, "UPDATE avatar set TotalXP = TotalXP + %i WHERE AvatarGUID = %lu;",m_dwExp_Value,who->m_pcAvatar->GetGUID( ));
			retcode2 = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand2, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode2 = SQLExecute( cDatabase::m_hStmt );
			retcode2 = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode2 = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			//check to see if we gained a level
			cMessage cmUpdateLevel = who->m_pcAvatar->UpdateLevel();
			who->AddPacket( WORLD_SERVER, cmUpdateLevel, 4);

			//Update level in DB
			char		szCommand3[512];
			RETCODE		retcode3;
			sprintf( szCommand3, "UPDATE avatar set Level = %i WHERE AvatarGUID = %lu;",who->m_pcAvatar->m_cStats.m_dwLevel,who->m_pcAvatar->GetGUID( ));
			retcode3 = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand3, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode3 = SQLExecute( cDatabase::m_hStmt );
			retcode3 = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode3 = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			//Test to update Pyreals...seems to work.
			cMessage GivePy = who->m_pcAvatar->UpdatePyreals(100,1);
			who->AddPacket( WORLD_SERVER, GivePy, 4);
			
			// It works :)
			cMonsterDeathParam* MonsterDeathParam = new cMonsterDeathParam( m_dwGUID, m_dwMonsterModelID, who->m_pcAvatar->GetGUID() );
			int iJob = cMasterServer::m_pcJobPool->CreateJob( &cMonster::DeathAnimation, (LPVOID) MonsterDeathParam, NULL, "MonsterDeath", 20, 1 );
		}
		
		SimpleAI::SetAttackComplete( m_dwGUID );
	}	
}

/**
 *	Handles the death animation for a killed monster.
 */
int cMonster::DeathAnimation( LPVOID wp, LPVOID lp )
{
	cMonsterDeathParam* MonsterDeathParam = (cMonsterDeathParam *) wp;

	cObject* pcMonster = cWorldManager::FindObject( MonsterDeathParam->GetMonsterGUID() );
	cModels* pcModel = cModels::FindModel( MonsterDeathParam->GetMonsterModelID() );
	cClient* pcClient = cClient::FindClient( MonsterDeathParam->GetClientGUID() );

	if(pcModel)
	{
		char CorpseName[100];
		char KilledBy[255];
		//Name of the corpse
		sprintf(CorpseName,"Corpse of %s",pcMonster->m_strName.c_str( ));
		//Who killed it (it's description)
		sprintf(KilledBy,"Killed by %s",pcClient->m_pcAvatar->m_strName.c_str());
		
		cCorpse* moncor = new cCorpse(pcModel->m_dwModelNumber,cWorldManager::NewGUID_Object(),pcMonster->m_Location,CorpseName,KilledBy);
		moncor->SetData(pcModel->m_wAnimConfig,pcModel->m_wSoundSet,pcModel->m_wModel,WORD(0x1070),MonsterDeathParam->GetMonsterModelID(),pcModel->m_flScale);
		
		//Add a copy of the monster as a corpse.
		cWorldManager::AddObject(moncor,true);
		
		//Make it disappear after 120 seconds.
		CorpseCleaner::AddCorpse(moncor->GetGUID(),120,0);
		
		//Remove the real monster
		cMessage cmRemModel;
		cmRemModel << 0xF747L << MonsterDeathParam->GetMonsterGUID() << DWORD( 1 );
		cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, cmRemModel, 3 );
		SimpleAI::RemoveMonster(MonsterDeathParam->GetMonsterGUID());
		cWorldManager::RemoveObject( pcMonster );
	}

	SAFEDELETE( MonsterDeathParam )

	return -1;
}

/**
 *	Handles the actions of monster objects.
 *
 *	This member function is called whenever a monster should perform an action.
 */
void cMonster::Action(cClient* who)
{
	// Need to figure out action types for Monsters
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << ++who->m_dwF7B0Sequence << 0x01C7L << 0L;
	who->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the assessment of monster objects.
 *
 *	This member function is called whenever a monster is assessed by a client.
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cMonster::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	WORD wAnimation;
	wAnimation = 0x54L;
	int iRand = rand();
	iRand = (iRand > 6400) ? iRand % 6400 : iRand;
	iRand = (iRand < 1600) ? 1600 + iRand : iRand;
	iRand = iRand/2500;

	DWORD dwSuccess;
	if(iRand > 1)
	{
		dwSuccess = 0x1L;
	}
	else
	{
		dwSuccess = 0x0L;
	}

	DWORD dwCharacterFlags; // Needs Decoding
	dwCharacterFlags = 0x0CL; // 0x04 - Level, Health | 0x08 - Stats

	cmAssess	<< 0xF7B0L 
				<< pcAssesser->m_pcAvatar->GetGUID( ) 
				<< ++pcAssesser->m_dwF7B0Sequence 
				<< 0xC9L 
				<< m_dwGUID 
				<< 0x100L // Flags 0x00000100 - Character Data
				<< dwSuccess;//0x1L;
	cModels *pcModel = cModels::FindModel( m_dwMonsterModelID );

	if( pcModel )
	{
		m_dwSpecies = pcModel->m_dwSpecies;
	}
	// Process the Flag Specified Data
	cmAssess	<< 	dwCharacterFlags
				// Mask 0x04
				<< m_dwLevel
				<< m_dwCurrenthealth
				<< m_dwMaxHealth
				// Mask 0x08
				<< m_dwStr
				<< m_dwEnd
				<< m_dwQuick
				<< m_dwCoord
				<< m_dwFocus
				<< m_dwSelf
				<< m_dwCurrentStamina
				<< m_dwCurrentMana
				<< m_dwMaxStamina
				<< m_dwMaxMana
				// End Mask
				<< m_dwSpecies;

	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
	// The Action has been completed
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

/**
 *	Handles the message sent for clients' melee attacking of monsters.
 *
 *	This member function is called whenever a client physically attacks a monster.
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type Inflict Melee damage (0x000001B1).
 */
cMessage cMonster::DoDamageMessage(DWORD F7B0seq,std::string target,DWORD damagetype,double severity,DWORD amount)
{
	cMessage cmRet;
	cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x01B1L << target.c_str() << damagetype << severity << amount;
	return cmRet;
}

/**
 *	Handles the message sent for the melee attacking of clients by monsters.
 *
 *	This member function is called whenever a client is physically attacked by a monster.
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type Receieve Melee damage (0x000001B2).
 */
cMessage cMonster::RecieveDamageMessage(DWORD F7B0seq,std::string giver,DWORD damagetype,double severity,DWORD amount,DWORD location)
{
	cMessage cmRet;
	cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x01B2L << giver.c_str() << damagetype << severity << amount << location;
	return cmRet;
}

/**
 *	Handles the message sent for the updating of creatures' health bars.
 *
 *	This member function is called whenever a monster's health should be updated for a client.
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type Update Health (0x000001C0).
 */
cMessage cMonster::DecrementHealth(DWORD dwGUID, WORD amount,signed int &newhealth)
{
	m_dwCurrenthealth -= amount;
	newhealth = m_dwCurrenthealth;
	cMessage cmRet;
	//cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x1C0L << m_dwGUID << static_cast<float> (m_dwCurrenthealth);
	return cmRet;
}

cMessage cMonster::SetHealth(DWORD dwNewHealth)
{
	m_dwCurrenthealth = static_cast<signed int>(dwNewHealth);
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x2L << dwNewHealth;
	return cmRet;
}

/**
 *	Handles the message sent for the setting of creatures' health bars.
 *
 *	This member function is called when a monster's health should be altered for a client.
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type Update Health (0x000001C0).
 */
cMessage cMonster::AdjustBar(DWORD dwGUID, DWORD F7B0Sequence )
{
	cMessage cmHealthLoss;
	double flHealthStat = ((( double )100/ m_dwMaxHealth)* m_dwCurrenthealth)/100;
	
	cmHealthLoss << 0xF7B0L << dwGUID << F7B0Sequence << 0x01C0L << m_dwGUID << float(flHealthStat);
	return cmHealthLoss;
}

cMessage cMonster::ChangeCombatMode(bool fMode )
{
	cMessage cmRet;
	cModels *pcModel = cModels::FindModel( m_dwMonsterModelID );

	unsigned char Canim4 [] = {
		0x4C, 0xF7, 0x00, 0x00, // 00 - 03
		0xE6, 0xD2, 0x09, 0x50, // 04 - 07
		0x3D, 0x00, 0x07, 0x00, // 08 - 11
		0x01, 0x00, 0x00, 0x00, // 12 - 15
		0x00, 0x00, 0x3C, 0x00, // 16 - 19
		0x01, 0x00, 0x00, 0x00, // 20 - 23
		0x3C, 0x00, 0x00, 0x00, // 24 - 27
	};
	CopyMemory(&Canim4[4],&m_dwGUID,4);
	CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
	m_wCurAnim++;
	CopyMemory(&Canim4[10],&m_wCurAnim,2);
	m_wMeleeSequence++;
	CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
	//CopyMemory(&Canim4[24],&pcModel->m_cAnimations.m_wStance[int(fMode)],2);
	cmRet.pasteData(Canim4,sizeof(Canim4));

	return cmRet;
}

cMessage cMonster::CombatAnimation(DWORD dwTarget, WORD wAttackAnim )
{
	cMessage cmRet;
	unsigned char Canimation [] = {
		0x4C, 0xF7, 0x00, 0x00, 
		0xE6, 0xD2, 0x09, 0x50, // object
		0x3C, 0x00, 0x18, 0x00, // numLogins, sequence
		0x03, 0x00, 0x00, 0x00, // numAnims, activity
		0x00, 0x01, 0x00, 0x00, // Anim type, type flag
		0xC1, 0x80, 0x99, 0xB1, 
		0x63, 0x00, 0x00, 0x00, 
		0x60, 0x00, 0x60, 0x00, 
		0x01, 0x00, 0xE4, 0x38, 
		0x8E, 0x3F, 0x00, 0x00, 
		0x20, 0x8B, 0x99, 0xB1, // dwTarget
	};
	CopyMemory(&Canimation[4],&m_dwGUID,4);
	CopyMemory( &Canimation[8], &m_wNumLogins, 2 );
	m_wCurAnim++;
	CopyMemory(&Canimation[10],&m_wCurAnim,2);
	m_wMeleeSequence++;
	CopyMemory(&Canimation[12],&m_wMeleeSequence,2);
	CopyMemory(&Canimation[0x28],&dwTarget,4);
	CopyMemory(&Canimation[0x20],&m_wCurAnim,2);
	CopyMemory(&Canimation[24],&wAttackAnim,2);
	cmRet.pasteData(Canimation,sizeof(Canimation));
	return cmRet;
}

cMessage cMonster::TurnToTarget( float flHeading, DWORD dwTargetGUID )
{
	cMessage cmTurnToTarget;

	cmTurnToTarget	<< 0xF74CL 
					<< m_dwGUID 
					<< m_wNumLogins
					<< ++m_wCurAnim
					<< ++m_wMeleeSequence
					<< WORD(0x0000)
					<< BYTE(0x08) 
					<< BYTE(0x00)
					<< WORD(0x003D)
					<< dwTargetGUID
					<< flHeading
					<< 0x1139EE0F
					<< 1.0f
					<< 0x00000000;

	return cmTurnToTarget;
}

cMessage cMonster::MoveToTarget( cClient *pcWho  )
{
	// pcWho is the target client 
	cMessage cmMoveToTarget;
	float flDistance;
	float flHeadingTarget;
	DWORD dwFlags;
		
	dwFlags = 0x1279EFF0;

	flHeadingTarget = GetHeadingTarget( pcWho->m_pcAvatar->m_Location.m_dwLandBlock,pcWho->m_pcAvatar->m_Location.m_flX,pcWho->m_pcAvatar->m_Location.m_flY,pcWho->m_pcAvatar->m_Location.m_flZ);
	flDistance = GetRange( pcWho->m_pcAvatar->m_Location.m_dwLandBlock, pcWho->m_pcAvatar->m_Location.m_flX, pcWho->m_pcAvatar->m_Location.m_flY, pcWho->m_pcAvatar->m_Location.m_flZ );

	cmMoveToTarget	<< 0xF74CL
					<< m_dwGUID	
					<< m_wNumLogins	
					<< ++m_wCurAnim	
					<< ++m_wMeleeSequence
					<< WORD(0x0000)
					<< BYTE(0x06)
					<< BYTE(0x00)
					<< WORD(0x003E);
					
	// Who/Where we are Moving to
	cmMoveToTarget	<< pcWho->m_pcAvatar->GetGUID( )
					<< m_Location.m_dwLandBlock
					<< m_Location.m_flX
					<< m_Location.m_flY
					<< m_Location.m_flZ
					<< dwFlags
					<< 0.0f
					<< 0.1f 
					<< 75.0f
					<< 1.0f
					<< 15.0f
					<< flHeadingTarget
					<< 1.8201754f;

	//cLocation tempLoc;

	//tempLoc = EstimateLoc(flHeadingTarget,10,flDistance,2.0f);

	//cWorldManager::MoveRemObject( this );
	//SetLocation(tempLoc.m_dwLandBlock,tempLoc.m_flX,tempLoc.m_flY,tempLoc.m_flZ,tempLoc.m_flA,tempLoc.m_flW);
	//cWorldManager::MoveAddObject( this );
	
	//flDistance = GetRange( pcWho->m_pcAvatar->m_Location.m_dwLandBlock, pcWho->m_pcAvatar->m_Location.m_flX, pcWho->m_pcAvatar->m_Location.m_flY, pcWho->m_pcAvatar->m_Location.m_flZ );
	//char Command2[100];
	//sprintf(Command2,"Range After Move: %f \r\n",flDistance);
	//UpdateConsole(Command2);
	
	return cmMoveToTarget;
}

//////////////////////

cMessage cMonster::MoveTarget( cClient *pcWho )
{
	// pcWho is the target client 
	cMessage cmMoveToTarget;
	float flHeadingTarget;
	DWORD dwFlags;
		
	dwFlags = 0x1279EFF0;

	flHeadingTarget = GetHeadingTarget(pcWho->m_pcAvatar->m_Location.m_dwLandBlock,pcWho->m_pcAvatar->m_Location.m_flX,pcWho->m_pcAvatar->m_Location.m_flY,pcWho->m_pcAvatar->m_Location.m_flZ);

	cmMoveToTarget	<< 0xF74CL
					<< m_dwGUID	
					<< m_wNumLogins	
					<< ++m_wCurAnim	
					<< ++m_wMeleeSequence
					<< WORD(0x0000)
					<< BYTE(0x06)
					<< BYTE(0x00)
					<< WORD(0x003E);

	cmMoveToTarget	<< pcWho->m_pcAvatar->GetGUID( )
					<< pcWho->m_pcAvatar->m_Location.m_dwLandBlock
					<< pcWho->m_pcAvatar->m_Location.m_flX
					<< pcWho->m_pcAvatar->m_Location.m_flY
					<< pcWho->m_pcAvatar->m_Location.m_flZ
					<< dwFlags
					<< 0.0f
					<< 0.1f 
					<< 75.0f
					<< 1.0f
					<< 15.0f
					<< flHeadingTarget
					<< 1.8201754f;
			
	cLocation tempLoc;
		
	//cWorldManager::MoveRemObject( this );
	//SetLocation(&pcWho->m_pcAvatar->m_Location);
	//cWorldManager::MoveAddObject( this );

	return cmMoveToTarget;
}

//////////////////////////////////////////////

cMessage cMonster::ReturnToSpawn( )
{
	cMessage cmReturnToSpawn;
	float flHeadingTarget = 0.0f;
	DWORD dwFlags;
	float flDistance = 1.0f;
	
	dwFlags = 0x1291EE0F;

	flHeadingTarget = GetHeadingTarget(m_SpawnLoc.m_dwLandBlock,m_SpawnLoc.m_flX,m_SpawnLoc.m_flY,m_SpawnLoc.m_flZ);
	flDistance = GetRange(m_SpawnLoc.m_dwLandBlock, m_SpawnLoc.m_flX, m_SpawnLoc.m_flY, m_SpawnLoc.m_flZ );

	cLocation tempLoc;
	tempLoc = EstimateLoc(flHeadingTarget,10,flDistance,2.5f);

	cmReturnToSpawn	<< 0xF74CL	
					<< m_dwGUID
					<< m_wNumLogins
					<< ++m_wCurAnim	
					<< ++m_wMeleeSequence
					<< WORD(0x0000)	
					<< BYTE(0x07)
					<< BYTE(0x00)
					<< WORD(0x003D);
					
	// Animation Type 0x07
	cmReturnToSpawn	<< tempLoc.m_dwLandBlock	
					<< tempLoc.m_flX
					<< tempLoc.m_flY
					<< tempLoc.m_flZ
					<< dwFlags
					<< 0.0f
					<< 1.0f
					<< DWORD(0x7F7FFFFF)
					<< 0.8f	// Speed
					<< 5.0f
					<< flHeadingTarget
					<< 1.5f;//1.6346154f;

	//cWorldManager::MoveRemObject( this );
	//SetLocation(tempLoc.m_dwLandBlock,tempLoc.m_flX,tempLoc.m_flY,tempLoc.m_flZ,tempLoc.m_flA,tempLoc.m_flW);
	//cWorldManager::MoveAddObject( this );
		
	//flDistance = GetRange(m_SpawnLoc.m_dwLandBlock, m_SpawnLoc.m_flX, m_SpawnLoc.m_flY, m_SpawnLoc.m_flZ );	char Command2[100];
	//sprintf(Command2,"Range After Move: %f \r\n",flDistance);
	//UpdateConsole(Command2);

	return cmReturnToSpawn;
}

cMessage cMonster::SetPosition()
{
	cMessage cmSetPosition;
	DWORD dwFlags = 0x34L;

	cmSetPosition	<< 0xF748L
					<< m_dwGUID	
					<< 0x34L;

	cmSetPosition	<< m_Location.m_dwLandBlock
					<< m_Location.m_flX
					<< m_Location.m_flY
					<< m_Location.m_flZ
					<< m_Location.m_flA
					//<< m_Location.m_flB
					//<< m_Location.m_flC
					<< m_Location.m_flW;

	return cmSetPosition;
}