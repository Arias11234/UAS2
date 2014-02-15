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
 *	@file Objects.cpp
 *	Implements general functionality for all objects.
 *	All object types inherit from this class.
 */

#include "Client.h"
#include "DatFile.h"
#include "Object.h"
#include "WorldManager.h"
#include <algorithm>

void cObject::SetLocation( double dNS, double dEW )
{
	DWORD dwNewLandBlock;

	m_Location.m_flX	= ( dEW * 10.0f + 1020.0f ) * 24.0f;
	m_Location.m_flY	= ( dNS * 10.0f - 1020.0f ) * 24.0f;
	m_Location.m_flZ	= 500.0f;
	dwNewLandBlock		=	( (BYTE) ((((DWORD)m_Location.m_flX % 192) / 24.0f) * 8.0f) + 
							( ((DWORD)m_Location.m_flY % 192) / 24) ) |
							( (0x00) << 8) |
							( (BYTE) (m_Location.m_flY / 192.0f) << 16) |
							( (BYTE) (m_Location.m_flX / 192.0f) << 24);
	dwNewLandBlock		-= 0x00010000;
	m_Location.m_flX	= (float)((int) m_Location.m_flX % 192);
	m_Location.m_flY	= (float)((int) m_Location.m_flY % 192);

	m_Location.m_dwLandBlock = dwNewLandBlock;
}

void cObject::ReSpawn( cObject *pcObject )
{

}

cMessage cObject::Animation( WORD wAnim, float flPlaySpeed )
{
	cMessage cAnim;
	WORD wAnimPart2;

	static BYTE Canimation[] = {
		0x4C, 0xF7, 0x00, 0x00, 0x56, 0x1F, 0x05, 0x50, 0x3C, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x3D, 0x00, 0x80, 0xF0, 0xD0, 0x03, 0x7C, 0x00, 0x01, 0x00, 0xCA, 0x3F, 0x00, 0x00,//0x00, 0x00, 0x80, 0x3F, 
	};

	++m_wCurAnim;

	CopyMemory( &Canimation[04], &m_dwGUID, 4 );
	CopyMemory( &Canimation[8],  &m_wNumLogins, 2 );
	CopyMemory( &Canimation[10], &m_wCurAnim, 2 );
	CopyMemory( &Canimation[12], &++m_wMeleeSequence, 2 );
	CopyMemory( &Canimation[24], &wAnim, 2 );
	CopyMemory( &Canimation[28], &flPlaySpeed, 4 );

	wAnimPart2 = m_wCurAnim - 6;
	CopyMemory( &Canimation[26], &wAnimPart2, 2 );
	
	cAnim.CannedData( Canimation, sizeof( Canimation ) );
	return cAnim;
}

cMessage cObject::Animation( WORD wAnim, float flPlaySpeed, BYTE bActivity  )
{
	cMessage cAnim;
	WORD wAnimPart2;

	static BYTE Canimation[] = {
		0x4C, 0xF7, 0x00, 0x00, 0x56, 0x1F, 0x05, 0x50, 0x3C, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x3D, 0x00, 0x80, 0xF0, 0xD0, 0x03, 0x7C, 0x00, 0x01, 0x00, 0xCA, 0x3F, 0x00, 0x00,//0x00, 0x00, 0x80, 0x3F, 
	};

	++m_wCurAnim;

	CopyMemory( &Canimation[04], &m_dwGUID, 4 );
	CopyMemory( &Canimation[8],  &m_wNumLogins, 2 );
	CopyMemory( &Canimation[10], &m_wCurAnim, 2 );
	CopyMemory( &Canimation[12], &++m_wMeleeSequence, 2 );
	CopyMemory( &Canimation[24], &wAnim, 2 );
	CopyMemory( &Canimation[28], &flPlaySpeed, 4 );

	wAnimPart2 = m_wCurAnim - 6;
	CopyMemory( &Canimation[26], &wAnimPart2, 2 );
	
	cAnim.CannedData( Canimation, sizeof( Canimation ) );
	return cAnim;
}

void cObject::Attack(cClient *pcAttacker, float flDamageSlider, DWORD F7B0Sequence )
{

}

void cObject::SpellAttack(cClient *pcAttacker, cObject *pcWarSpell, DWORD F7B0Sequence )
{

}

cMessage cObject::RemoveObj( DWORD dwGUID )
{
	cMessage cmRemove;
	
	cmRemove << 0xF747L << dwGUID << 0x0L ;
	return cmRemove;
}

cMessage cObject::AdjustBar( DWORD dwGUID, DWORD F7B0Sequence )
{
	cMessage cmHealth;
	
	cmHealth << 0xF7B0L << dwGUID << F7B0Sequence << 0x01C0L << m_dwGUID << 1.0f;
	return cmHealth;
}

cMessage cObject::LocationPacket( )
{
	WORD wInvSeq = (WORD(m_bInventorySequence) * 5) - 1;

	cMessage cmPacket;

	cmPacket	<< 0xF748L << m_dwGUID << 0x34L << m_Location.m_dwLandBlock << m_Location.m_flX << m_Location.m_flY
				<< m_Location.m_flZ << m_Location.m_flA << m_Location.m_flB << m_wNumLogins << WORD( wInvSeq ) << WORD( ++m_wPositionSequence ) << WORD( 0 );

	return cmPacket;
}

cMessage	cObject::CombatAnimation( DWORD dwTarget, WORD wAttackAnim )
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

cMessage	cObject::ChangeCombatMode( bool fMode)
{
	cMessage cmReturn;
	cmReturn << 0xFF00;
	return cmReturn;
}

/**
 *	Handles the message sent for the creation of an object in the world.
 *
 *	This function is called whenever an object should be created in the world for a client.
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cAbiotic::CreatePacket( )
{
	cMessage cmCreate;

	cmCreate << 0xF745L << m_dwGUID << BYTE( 0x11 ) << BYTE( 0 ) << BYTE( 0 ) << BYTE( 0 );
	
	DWORD dwFlags = 0x00001881;
	if ( !m_fIsOwned )
		dwFlags |= 0x8000;

	cmCreate << dwFlags;
	
	if( m_fIsSolid )
		cmCreate << DWORD( 0x00000008 );
	else
		cmCreate << DWORD( 0x00010044 );

	if ( !m_fIsOwned )
		cmCreate.CannedData( (BYTE *)&m_Location, sizeof( cLocation ) );
	
	cmCreate << 0x20000014L << 0x3400002BL;
	
	DWORD dwModel = 0x02000000L + m_dwModel;
	cmCreate << dwModel << m_flScale;
	
	// SeaGreens
	WORD wUnkFlag2 = 0x1;
	WORD wUnkFlag3 = 0x1;
	WORD wUnkFlag4 = 0;
	WORD wUnkFlag6 = 0;
	WORD wUnkFlag7 = 0;
	WORD wUnkFlag8 = 0;
	WORD wUnkFlag10 = 0;
	
	cmCreate	<< m_wPositionSequence 
				<< wUnkFlag2 
				<< wUnkFlag3 
				<< wUnkFlag4 
				<< m_wNumPortals 
				<< wUnkFlag6 
				<< wUnkFlag7 
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;

	DWORD dwFlags2 = 0x00000218;
	if ( m_fIsOwned )
		dwFlags2 |= 0x4000;
	
	cmCreate << dwFlags2 << m_strName.c_str( ) << WORD( 0x013A ) << WORD( m_wIcon );
				
	DWORD dwObjectFlags1 = 0x01;	
	cmCreate << dwObjectFlags1;
	
	DWORD dwObjectFlags2 = 0x00;
	if ( m_fSelectable )	dwObjectFlags2 |= 0x10;
	else					dwObjectFlags2 |= 0x80;
	if ( !m_fEquippable )	dwObjectFlags2 |= 0x04;	// Uncommented Blackstaff 04/13/02
	else					dwObjectFlags2 |= 0x00;
	//dwObjectFlags2 |= 0x10000 | 0x20000 | 0x40000;

	cmCreate << dwObjectFlags2 << m_dwValue << m_dwValue;// << 0x00100000 << 0x00100000 << 0x00100000;
	
	BYTE bEquipType = 0x01;
	cmCreate << bEquipType;
	
	if ( m_fIsOwned )
		cmCreate << m_dwContainer;
	
	return cmCreate;
}

/**
 *	Handles the assessment of objects.
 *
 *	This function is called whenever an objects is assessed by a client.
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cAbiotic::Assess( cClient *pcAssesser )
{
	cMessage cmAssess;
	cmAssess << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID() << ++pcAssesser->m_dwF7B0Sequence << 0xC9L;
	cmAssess << m_dwGUID << 0x00000800L << 1L << 1L << m_dwValue << m_dwWeight;
	pcAssesser->AddPacket( WORLD_SERVER, cmAssess, 4 );
	
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x01C7L << 0L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

DWORD cObject::CalculateDamage(int strength, float flPower, float flResistance )
{	
	srand( timeGetTime( ) );

	int iRand = rand( );
	iRand = (iRand > 6400) ? iRand % 6400 : iRand;
	iRand = (iRand < 1600) ? 1600 + iRand : iRand;
	
	double dLuckFactor = 3200.0f / (double)iRand; 

	// MaxDamage = (Strength * 0.2 * (flPower + 1.15)) / (flResistance + 5.0)
	double dMaxDamage = (double)((strength * 0.2f) * ((flPower + .25f) * 4)) / (double)(flResistance + 7.5f);
	
	return dMaxDamage * dLuckFactor + (rand( ) % 2);
}

cMessage cObject::TurnToTarget( float flHeading, DWORD dwTargetGUID )
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

float cObject::GetRange( DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ )
{
	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float flRange;
	lb LocLB, TarLB;

	CopyMemory( &LocLB,&m_Location.m_dwLandBlock , 4 );
	CopyMemory( &TarLB,&dwTargetLandblock , 4 );
	
	nsCoord = ( ( ( ( LocLB.bLo + 1 ) * 8 ) + ( m_Location.m_flY / 24 ) ) - 1027.5 )/10;
	ewCoord = ( ( ( ( LocLB.bHi + 1 ) * 8 ) + ( m_Location.m_flX / 24 ) ) - 1027.5 )/10;
	  
	nsTarCoord = ( ( ( ( TarLB.bLo + 1 ) * 8 ) + ( flTarY / 24 ) ) - 1027.5 )/10;
	ewTarCoord = ( ( ( ( TarLB.bHi + 1 ) * 8 ) + ( flTarX / 24 ) ) - 1027.5 )/10;

	flRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2))*10;

	return flRange;
}

float cObject::GetHeadingTarget( DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ )
{
	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float flHeading;
	float intRange;
	lb LocLB, TarLB;
	flHeading = 0.0f;

	CopyMemory( &LocLB,&m_Location.m_dwLandBlock , 4 );
	CopyMemory( &TarLB,&dwTargetLandblock , 4 );
/*	
	nsCoord = ( ( ( ( LocLB.bLo + 1 ) * 8 ) + ( flY / 24 ) ) - 1027.5 )/10;
	ewCoord = ( ( ( ( LocLB.bHi + 1 ) * 8 ) + ( flX / 24 ) ) - 1027.5 )/10;
	  
	nsTarCoord = ( ( ( ( TarLB.bLo + 1 ) * 8 ) + ( flTarY / 24 ) ) - 1027.5 )/10;
	ewTarCoord = ( ( ( ( TarLB.bHi + 1 ) * 8 ) + ( flTarX / 24 ) ) - 1027.5 )/10;
*/
	
	nsCoord = (((((m_Location.m_dwLandBlock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(m_Location.m_flY / 24) - 1027.5; 
	ewCoord = (((((m_Location.m_dwLandBlock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(m_Location.m_flX / 24) - 1027.5;
	nsTarCoord = (((((dwTargetLandblock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(flTarY / 24) - 1027.5; 
	ewTarCoord = (((((dwTargetLandblock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(flTarX / 24) - 1027.5;

	intRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2));
	if(intRange > 0)
	{
	  if(nsTarCoord - nsCoord < 0 )
	  { 
		  flHeading = acos((ewTarCoord - ewCoord) / intRange) * 57.2957796; 
	  } 
	  else 
	  { 
		  flHeading = acos(-(ewTarCoord - ewCoord) / intRange) * 57.2957796 + 180; 
	  } 
	}
	return flHeading + 90;
}

cMessage cObject::MoveToTarget( cClient *pcWho  )
{
	// pcWho is the target client 
	cMessage cmMoveToTarget;
	float flHeadingTarget;
	DWORD dwFlags;
		
	dwFlags = 0x1279EFF0;

	flHeadingTarget = GetHeadingTarget( pcWho->m_pcAvatar->m_Location.m_dwLandBlock,pcWho->m_pcAvatar->m_Location.m_flX,pcWho->m_pcAvatar->m_Location.m_flY,pcWho->m_pcAvatar->m_Location.m_flZ);

																	// Positional Info only
	cmMoveToTarget	<< 0xF74CL										// dword 1
					<< m_dwGUID										// dword 2
					<< m_wNumLogins									// word
					<< ++m_wCurAnim									// + word = dword 3
					<< ++m_wMeleeSequence							// word
					<< WORD(0x0000)									// + word = dword 4
					<< BYTE(0x06)									// byte
					<< BYTE(0x00)									// +byte
					<< WORD(0x003E);								// +word = dword 5
					
	// Who/Where we are Moving to
	cmMoveToTarget	<< pcWho->m_pcAvatar->GetGUID( )				// dword 6
					<< m_Location.m_dwLandBlock//pcWho->m_pcAvatar->m_Location.m_dwLandBlock	// dword 7
					<< m_Location.m_flX//pcWho->m_pcAvatar->m_Location.m_flX			// dword 8
					<< m_Location.m_flY//pcWho->m_pcAvatar->m_Location.m_flY			// dword 9
					<< m_Location.m_flZ//pcWho->m_pcAvatar->m_Location.m_flZ			// dword 10
					<< dwFlags										// dword 11 - does not change
					<< flHeadingTarget								// dowrd 12
					<< 0.1f 										// dword 13 - does not change //pcWho->m_pcAvatar->m_Location.m_flC
					<< 15.0f										// dword 14 - Run speed value
					<< 1.5f											// dword 15
					<< 15.0f										// dword 16
					<< 0x0L											// dword 17
					<< 1.8201754f;									// dword 18

	//SetLocation(&pcWho->m_pcAvatar->m_Location);

	return cmMoveToTarget;
}
//////////////////////

cMessage cObject::MoveTarget( cClient *pcWho )
{
	// pcWho is the target client 
	cMessage cmMoveToTarget;
	float flHeadingTarget;
	DWORD dwFlags;
		
	dwFlags = 0x1279EFF0;

	flHeadingTarget = GetHeadingTarget( pcWho->m_pcAvatar->m_Location.m_dwLandBlock,pcWho->m_pcAvatar->m_Location.m_flX,pcWho->m_pcAvatar->m_Location.m_flY,pcWho->m_pcAvatar->m_Location.m_flZ);

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

	SetLocation(&pcWho->m_pcAvatar->m_Location);
			
	return cmMoveToTarget;
}

////////////////////////////////////////////
// Never Use this function
////////////////////////////////////////////
cMessage cObject::ReturnToSpawn(  )
{
	cMessage cmReturnToSpawn;
	float flHeadingTarget = 0.0f;
	DWORD dwFlags;
	float flDistance = 1.0f;
	cLocation tempLoc;
	dwFlags = 0x1291EE0F;


	flHeadingTarget = GetHeadingTarget( m_SpawnLoc.m_dwLandBlock,m_SpawnLoc.m_flX,m_SpawnLoc.m_flY,m_SpawnLoc.m_flZ);
	flDistance = GetRange( m_SpawnLoc.m_dwLandBlock, m_SpawnLoc.m_flX, m_SpawnLoc.m_flY, m_SpawnLoc.m_flZ );
	tempLoc = EstimateLoc(flHeadingTarget,10,flDistance,2.0f);

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
					<< 0.0f//flHeadingTarget
					<< 1.0f
					<< DWORD(0x7F7FFFFF)
					<< 1.0f	
					<< 5.0f
					<< flHeadingTarget
					<< 1.6346154f;
			
	return cmReturnToSpawn;
}

///////////////////////////////////////////////////////////////////////////
// CoordLoc - Converts NS/EW map coordinates into Landblock Data form
// 
//
///////////////////////////////////////////////////////////////////////////
cLocation cObject::CoordLoc( float dNS, float dEW )
{
	cLocation Loc;
	char tmpLB[5];
	struct{
		BYTE bHi;
		BYTE bLo;
		BYTE bOO;
		BYTE bLow;
	} lb;

	Loc.m_flX = ((dEW * 10.0f) + 1027.5f ) * 24.0f;

	Loc.m_flY = ((dNS * 10.0f) + 1027.5f ) * 24.0f;
	Loc.m_flZ = 0.0f;//500.0f;

	Loc.m_flX = ( float )( ( DWORD ) Loc.m_flX % 192);
	Loc.m_flY = ( float )( ( DWORD ) Loc.m_flY % 192);

	sprintf(tmpLB,"    \r\n");
	lb.bHi = ((((dEW*10)+1027.5)-(Loc.m_flX/24))/8);
	lb.bLo = ((((dNS*10)+1027.5)-(Loc.m_flY/24))/8)-1;
	lb.bOO = 0x00; // Indicates on Surface map   0x01 indicates in Dungeon
	lb.bLow = ((Loc.m_flX/24)*8) + (Loc.m_flY/24);

	CopyMemory( &tmpLB[3], &lb.bHi, 1 );
	CopyMemory( &tmpLB[2], &lb.bLo, 1 );
	CopyMemory( &tmpLB[1], &lb.bOO, 1 );
	CopyMemory( &tmpLB[0], &lb.bLow, 1 );
	CopyMemory( &Loc.m_dwLandBlock, &tmpLB, 4 );

	return Loc;
}

cLocation	cObject::EstimateLoc( float flHeading, float flSpeed, float flDistToTarget, float flTime )
{
	cLocation Loc;
	lb LocLB,NewLB;
	
	float flEndX = 1.0f;
	float flEndY = 1.0f;
	float flRadHeading = 0.0f;
	float flDistance = 1.0f;
	float dNS, dEW;
	
	if(flSpeed < 10.0f)
	{ 
		flSpeed = 10.0f;
	}

	flDistance = (flSpeed*flTime);
	if( flDistToTarget < 0.5f)
	{
		flDistance = flDistToTarget/4;
	}
	
	flRadHeading = ((flHeading * 3.14159265359)/180);
	CopyMemory( &LocLB,&m_Location.m_dwLandBlock , 4 );


	 //dNS2 = ( ( ( ( LocLB.bLo + 1 ) * 8 ) + ( flY / 24 ) ) - 1027.5 )/10;
	 //dEW2 = ( ( ( ( LocLB.bHi + 1 ) * 8 ) + ( flX / 24 ) ) - 1027.5 )/10;

	flEndX = (float)(flDistance * sin(flRadHeading));
	flEndY = (float)(flDistance * cos(flRadHeading));

	dNS = (float)(m_Location.m_flY + flEndY);
	dEW = (float)(m_Location.m_flX + flEndX);

	// set Landblock HI (EW movement)
	if(dNS > 193)
	{
		NewLB.bLo = LocLB.bLo + 1;
		dNS = dNS - 193;
	}
	else if (dNS < 0)
	{
		NewLB.bLo = LocLB.bLo - 1;
		dNS = dNS + 193;
	}
	else 
	{
		NewLB.bLo = LocLB.bLo;
	}
	// Set Landblock LO (NS movement)
	if(dEW > 193)
	{
		NewLB.bHi = LocLB.bHi + 1;
		dEW = dEW - 193;
	}
	else if(dEW < 0)
	{
		NewLB.bHi = LocLB.bHi - 1;
		dEW = dEW + 193;
	}
	else
	{
		NewLB.bHi = LocLB.bHi;
	}

	Loc.m_flX = dEW;
	Loc.m_flY = dNS;
	Loc.m_flZ = 0.0f;
	NewLB.bOO = 0x00; // Indicates on Surface map   0x01 indicates in Dungeon
	NewLB.bLow = ((dEW/24)*8) + (dNS/24);

	CopyMemory( &Loc.m_dwLandBlock,&NewLB , 4 );

	return Loc;
}

cMessage cObject::SetPosition()
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
					<< m_Location.m_flB
					<< m_Location.m_flC
					<< m_Location.m_flW;

	return cmSetPosition;
}
