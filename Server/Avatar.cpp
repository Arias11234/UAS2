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
*

/**
 *	@file Avatar.cpp
 *	Implements general functionality for avatars (world characters).
 *
 *	Functionality includes messages sent when logging in an avatar, as well as updating 
 *	attributes, vitals, skills, experience, inventory, housing information, etc.
 *
 *	Loading of the actual avatar data is performed by the cDatabase class.
 *
 *	Inherits from the cObject class.
 */


#include <time.h>

#include "Avatar.h"
#include "CannedPackets.h"
#include "Database.h"
#include "DatFile.h"
#include "WorldManager.h"
#include "math.h"
#include "SimpleAI.h"
#include "MasterServer.h"
#include "Job.h"
#include "cSpell.h"

/*
Cubem0j0 -- Asheron's Call Formulas:

	Combat Burden (how much Stamina you use when attacking) = shield_burden + (1.5 * weapon_burden)
	Overall Burden		= 150bu per point of Strength
	Natural Resistance	= 
	Natural Regeneration= 
	Healing Rate = 2 * (max_health - current_health) with 50% base.
		Example: 200 max, 50 current: 2 * 150 = 300
		A base 300 skill has a 50% chance to heal.
	
	Armor Rending:		Percentage of armor ignored	= (skill - 160) / 400
	Critical Strike:	Percentage critical hits	= (skill - 100) / 600
	Crippling Blow:		Critical damage multiplier	= (skill - 40) / 60
	*Maximum Imbue Benefit*
		Armor Rending		- 60% armor ignored
		Critical Strike		- 50% critical strikes
		Crippling Blow		- (6 * damage_multiplier) on critial strikes
		Resistance Rending	- 150% damage (same as a Level 6 Life Vulnerability spell)
	
	(Damage over Time = (maximum damage)*((1-critical hit chance)*(2-variance)/2 + (critical hit chance)*(critical hit multiplier))*(Strength/Coordination modifier)*(Bow/Crossbow/Thrown Weapon modifier)*(rate-of-fire))
	Maximum Damage = Weapon's Max Damage + Blood Drinker (+ Unarmed Combat modifier)
		Excludes bonuses such as Heart Seeker or weapon modifications (UA / 10 for Phantom Katar)
	Critical hit chance = 0.10 for most weapons.
		For melee weapons with Critical Strike:		(base_skill - 100) / 600
		For missile weapons with Critical Strike:	(base_skill - 60) / 600
	Critical hit multiplier  = 2 for most weapons.
		Some "old-school" quest weapons had higher values.
		For melee weapons:		(base_skill - 40) / 60
		For missile weapons:	(base_skill) / 60
	Strength/Coordination Modifier
		(Bow, Crossbow, and Dagger use Coordination; Thrown Weapons and all other melee weapons use Strength.)
		Unarmed Combat, Bow, Crossbow, Thrown Weapons:  1 + (attribute - 55) * .008
		Other weapons:									1 + (attribute - 55) * .011
	Unarmed Combat Modifier = (base_skill / 20)
	Bow/Crossbow/Thrown Weapon Modifier = (100 + weapon_modifier) / 100 * power_modifier = 1.5 for most weapons (.5 is used for Hollow and Phantom melee weapons)
*/

/*
The character model:

	Characters are composed of palettes (colors), textures (designs), and models (shapes).
  
	All of this character model (palette, texture, and model) information is stored in the portal.dat.
		The portal.dat is separated into groups of files:
			Files prefixed 0x0400xxxx are color look-up tables (CLUTs), used to determine palette information.
			Files prefixed 0x0500xxxx are textures, used to determine texture/design information.
			Files prefixed 0x0100xxxx are models, used to determine model/shape information.

	The default character palette is file 0x0400007E.
	The default character complex model (which consists of textures and models) is:
		0x02000001 for male characters
		0x0200004E for female characters

	The palette information for a character has three parts:
		1. Palette		WORD (palette portal.dat entry minus 0x04000000)
		2. Offset		BYTE (number of palette entries to skip)
		3. Length		BYTE (number of palette entries to copy)

	The texture information for a character has three parts:
		1. Index		BYTE (the index of the model whose texture the new texture is replacing)
		2. Old Texture	WORD (texture portal.dat entry minus 0x05000000)
		3. New Texture	WORD (texture portal.dat entry minus 0x05000000)

	The model information for a character has two parts:
		1. Index		BYTE (the index of the model being replaced)
		2. Model		WORD (model portal.dat entry minus 0x01000000)

	By default, characters have three palettes, four textures, and seventeen models.
		
		The three palettes are:
			1. Skin Color (palettes 0 - 23)
			2. Hair Color (palettes 24 - 31)
			3. Eye Color (palettes 32 - 39)

		The four textures are:
			1. Hair (index 0x10, a range of textures)
			2. Forehead (index 0x10, a range of textures)
			3. Nose (index 0x10, a range of textures)
			4. Chin (index 0x10, a range of textures)

		The seventeen models (which constitute the seventeen parts of the character model) are:

	   Number	Index	Body Part
		 1.		0x00	Waist
		 2.		0x01	Left Upper Leg
		 3.		0x02	Left Lower leg
		 4.		0x03	Left Shin
		 5.		0x04	Left Foot
		 6.		0x05	Right Upper leg
		 7.		0x06	Right Lower Leg
		 8.		0x07	Right Shin
		 9.		0x08	Right Foot
		10.		0x09	Chest
		11.		0x0A	Upper Arm (shoulder) - Left
		12.		0x0B	Wrist - Left Arm
		13.		0x0C	Left Hand
		14.		0x0D	Upper Arm (shoulder) - Right
		15.		0x0E	Wrist - Right Arm
		16.		0x0F	Right Hand
		17.		0x10	Head

	Palettes are appended to the character's natural palettes and any other supplemental palettes.
		-Each palette consists of 255 distinct colors.
		-Beginning with the color at the "Offset" value, "Length" palettes are copied to the character's palette.
	
	Textures are appended to the character's natural textures and any other supplemental textures.
		-The "New Texture" entry is applied to the "Index" part of the character.
		-This information is appended to the rest of the character's texture information.

	Models replace the character's natural models and any supplemental models they cover.
		-The "Model" entry is applied to the "Index" part of the character.
		-This information replaces the information for the "Index" part of the character.

	The character palette is as follows (per information from Todd's Hacking Zone: http://todd.acdev.org/):
	   
	    Index Range	   Size			 Use
		  0 - 23  		24		Skin
		 24 - 31 		 8 		Hair
		 32 - 39 		 8 		Eyes
		 40 - 63		24 		Shirt
		 64 - 71		 8 		Pants
		 72 - 79		 8 		Abdomen, Leather
		 80 - 91		12 		Abdomen, Metal
		 92 - 95		 4 		Abdomen, Gold
		 96 - 107		12 		Lower Arm, Metal
		108 - 115		 8 		Lower Arm, Leather
		116 - 127		12 		Upper Arm, Metal
		128 - 135		 8 		Upper Arm, Leather
		136 - 151		16 		Leg, Metal
		152 - 159		 8 		Leg, Leather
		160 - 167		 8 		Feet
		168 - 173		 6 		Hands
		174 - 185 		12 		Leather Armor
		186 - 197		12 		Armor
		198 - 205		 8 		Armor
		206 - 215		10 		Armor
		216 - 239		24		Armor, Primary
		240 - 249		10		Helmet, Metal
		250 - 255		 6 		Helmet, Leather
*/

cMessage cAvatar::LocationPacket( )
{
	cMessage cMove;

	cMove	<< 0xF748L 
			<< GetGUID( )
			<< 0x34L
			<< m_Location.m_dwLandBlock
			<< m_Location.m_flX
			<< m_Location.m_flY
			<< m_Location.m_flZ
			<< m_Location.m_flA
			<< m_Location.m_flW
			<< WORD(m_wNumLogins)
			<< WORD(++m_wMoveCount)
			<< WORD(m_wPortalCount)
			<< WORD(0x0000);

	return cMove;
}

cMessage cAvatar::LoginCharacter()
{
	cMessage cmReturn;

	cmReturn << 0xF746L << GetGUID( );

	return cmReturn;
}

void cAvatar::LogoutCharacter()
{
	cFellowship* aFellowship;
	if (aFellowship = cFellowship::GetFellowshipByID(m_dwFellowshipID))
	{
		aFellowship->RemMember(GetGUID());

		SetFellowshipID(0);
		this->inFellow = false;
	}
}

/////////////////////////////////////////////////////////////////
// Huge Login Packet
/////////////////////////////////////////////////////////////////
/**
 *	Handles the message sent for the login of an avatar.
 *
 *	The message includes information on name, race, sex, class, levels, attributes, vitals, 
 *	skills, experience, deaths, burden, pyreals, spelltabs, inventory, and equipment.
 *	The function uses information loaded by cDatabase::LoadAvatarList. @see cDatabase::LoadAvatarList
 *
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type Login Character (0x00000013).
 */
cMessage cAvatar::CreateLoginPacket( DWORD F7B0seq )
{
	
	this->inFellow = false;
	this->SetFellowshipID(0);

	cMessage cmRet;
	cCharacterServer::m_dwClientCount++;
	//Cube:  UPDATE THIS SECTION
    cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x13L;

	DWORD dwLoginMask = 0x0000001B;
/*
	if (m_dwAllegianceID != 0)
	{
		dwLoginMask = dwLoginMask | 0x00000040;	//0040 - Allegiance info
	}
*/
	cmRet << dwLoginMask;
	cmRet << 0xAL;					//This should be # of slots equipped.	
	WORD numstats = 8;//8;
	cmRet << numstats << WORD( 0x40 );
	cmRet << 0x5L;
	DWORD dwBurden = 0;
	cmRet << dwBurden;
	cmRet << 0x14L;
	DWORD dwNumPyreals = 1000;
	cmRet << dwNumPyreals;
	cmRet << 0x15L;
	cmRet << m_dwTotalXP;
	cmRet << 0x16L;
	cmRet << m_dwUnassignedXP;
	cmRet << 0x18L;					//Skill credits
	cmRet << DWORD(m_bSkillCredits);
	cmRet << 0x19L;
	cmRet << m_cStats.m_dwLevel;
	cmRet << 0x28L << 0x1L;
//	cmRet << 0x2BL << m_dwDeaths;	//Total Deaths
	cmRet << 0x2FL << 0x0L;
	//more unknown vectors
	cmRet << WORD( 0x5 ) << WORD( 0x20 );
	cmRet << 4L << 0L << 0x2CL << 0L << 0x2DL << 0L << 0x2EL << 0L << 0x2FL << 0L;
	//String Vector
	cmRet << WORD( 0x4 ) << WORD( 0x10 );
	cmRet << 0x1L;
	cmRet << Name( );
	cmRet << 0x3L;

	cmRet << cDatabase::ReturnGender( m_wGender );

	cmRet << 0x4L;

	cmRet << cDatabase::ReturnRace( m_wRace );

    cmRet << 0x5L;
	cmRet << cDatabase::ReturnClass( m_wClass );

	cmRet << WORD( 0x1 ) << WORD( 0x20 );
		cmRet << 0x4L;
		cmRet << 0x30000000L;
	
	//Allegiance Info
	if (dwLoginMask & 0x00000040)
	{
		cAllegiance* aAllegiance;

		int numRecords = 0;
		std::list< Member >	memberList;

		if (aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID))
		{
			//Find the monarch's allegiance record
			if (aAllegiance->GetLeader() != m_dwGUID)
			{
				//Add the monarch's allegiance record
				Member memMonarch = aAllegiance->members.find(aAllegiance->GetLeader())->second;
				memberList.push_back( memMonarch );
				numRecords++;
			}

			//Find the player's allegiance record
			Member memPlayer = aAllegiance->members.find(m_dwGUID)->second;

			//Find the patron's allegiance record
			if (memPlayer.m_dwPatron != NULL)
			{
				Member memPatron = aAllegiance->members.find(memPlayer.m_dwPatron)->second;
				if (memPatron.m_dwGUID != aAllegiance->GetLeader())	//if the patron is not also the monarch
				{
					//Add the patron's allegiance record
					memberList.push_back( memPatron );
					numRecords++;
				}
			}
		}

		cmRet << WORD(numRecords);	//count of entries
		cmRet << WORD(0x0030);		//unknown (cannot be 0)
		for ( std::list<Member>::iterator list_iter = memberList.begin(); list_iter != memberList.end(); ++list_iter )
		{
			if ((*list_iter).m_dwGUID == aAllegiance->GetLeader())
				cmRet << DWORD(0x00000018);
			else
				cmRet << DWORD(0x00000019);
			cmRet	<< (*list_iter).m_dwGUID;	// DWORD	player GUID
		}
	
		memberList.clear();
	}

	/*cmRet << WORD( 0x1 ) << WORD( 0x10 );
	UCHAR corpseloc [] = {
	0x0E, 0x00, 0x00, 0x00, 0x28, 0x00, 0xB4, 0xA9, 
0x8B, 0x41, 0xC0, 0x42, 0x1A, 0x12, 0x3A, 0x43, 0xEB, 0xC9, 0x57, 0x42, 0xE9, 0x08, 0x2E, 0x3F, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0xBE, 0x3B, 0xBF};
	
	cmRet.pasteData( corpseloc, sizeof( corpseloc ) );*/

	DWORD loginmask2 = 0x103L; 
	cmRet << loginmask2;
	cmRet << 0x1L;
	cmRet << 0x1FFL;

	// Attributes
	for( int i=0; i < 6; i++ )
	{
		//k109:  Grab the base here so we can manipulate later...
		switch(i)
		{
			case 0:
				this->base_strength = this->m_cStats.m_lpcAttributes[i].m_dwCurrent;
				strength_exp_cost = m_cStats.m_lpcAttributes[i].m_dwXP;
				break;
			case 1:
				this->base_endurance = this->m_cStats.m_lpcAttributes[i].m_dwCurrent;
				endurance_exp_cost = m_cStats.m_lpcAttributes[i].m_dwXP;
				break;
			case 2:
				this->base_quickness = this->m_cStats.m_lpcAttributes[i].m_dwCurrent;
				quickness_exp_cost = m_cStats.m_lpcAttributes[i].m_dwXP;
				break;
			case 3:
				this->base_coordination = this->m_cStats.m_lpcAttributes[i].m_dwCurrent;
				coordination_exp_cost = m_cStats.m_lpcAttributes[i].m_dwXP;
				break;
			case 4:
				this->base_focus = this->m_cStats.m_lpcAttributes[i].m_dwCurrent;
				focus_exp_cost = m_cStats.m_lpcAttributes[i].m_dwXP;
				break;
			case 5:
				this->base_self = this->m_cStats.m_lpcAttributes[i].m_dwCurrent;
				self_exp_cost = m_cStats.m_lpcAttributes[i].m_dwXP;
				break;
		}
		cmRet << m_cStats.m_lpcAttributes[i].m_dwIncrement;
		cmRet << m_cStats.m_lpcAttributes[i].m_dwCurrent;
		//k109:  This needs to be updated with our new attr exp columns data...
		cmRet << m_cStats.m_lpcAttributes[i].m_dwXP;
	}

	//Vitals
	for(i = 0; i < 3; i++ )
	{
		switch(i)
		{
		case 0:
			base_health = m_cStats.m_lpcVitals[i].m_dwCurrent;
			health_exp_cost = m_cStats.m_lpcVitals[i].m_dwXP;
			break;
		case 1:
			base_stamina = m_cStats.m_lpcVitals[i].m_dwCurrent;
			stamina_exp_cost = m_cStats.m_lpcVitals[i].m_dwXP;
			break;
		case 2:
			base_mana = m_cStats.m_lpcVitals[i].m_dwCurrent;
			mana_exp_cost = m_cStats.m_lpcVitals[i].m_dwXP;
			break;
		}
		cmRet << m_cStats.m_lpcVitals[i].m_dwIncreases << 0L << m_cStats.m_lpcVitals[i].m_dwXP << m_cStats.m_lpcVitals[i].m_dwCurrent;
	}
	
	cmRet << WORD( 0x23 ) << WORD( 0x20 );
	
	char cSkillOrder [] = {
		0x20,0x21,0x01,0x22,0x02,0x23,0x03,0x24,0x04,0x25,0x05,0x26,
		0x06,0x27,0x07,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x12,
		0x13,0x14,0x15,0x16,0x17,0x18,0x1B,0x1C,0x1D,0x1E,0x1F
	};
	
	//Cube:  Continuing the work on skills here, adding m_dwXP to the loading of skills.
	for(i = 0; i < sizeof( cSkillOrder ); i++ )
	{
		cmRet << DWORD( m_cStats.m_lpcSkills[cSkillOrder[i]].m_wID );
		cmRet << WORD( m_cStats.m_lpcSkills[cSkillOrder[i]].m_dwIncreases );
		cmRet << WORD(0x0001);
		cmRet << DWORD( m_cStats.m_lpcSkills[cSkillOrder[i]].m_wStatus );
		cmRet << DWORD( m_cStats.m_lpcSkills[cSkillOrder[i]].m_dwXP );
		if ( m_cStats.m_lpcSkills[cSkillOrder[i]].m_wStatus == 3)
			cmRet << DWORD(0x0AL);
		else if ( m_cStats.m_lpcSkills[cSkillOrder[i]].m_wStatus == 2 )
			cmRet << DWORD(0x05L);
		else
			cmRet << DWORD(0x0L);
		cmRet << DWORD(0x0L);
		DWORD s_unk1 = 0x76A11716;
		DWORD s_unk2 = 0x418FB0A3;
		cmRet << s_unk1 << s_unk2;

		CalcSkill(m_cStats.m_lpcSkills[cSkillOrder[i]].m_wID);
	}

	//spell entries;
	cmRet << WORD( m_wSpellCount ) << WORD( 0x40 );
	for(i = 0;i < m_wSpellCount;i++)
	{
		cmRet << m_SpellBook[i].dwSpell_ID << m_SpellBook[i].flCharge << m_SpellBook[i].dwUnknownA << m_SpellBook[i].dwUnknownB ; 
	}
	cmRet << 0x4L;	//Login Mask 3
	cmRet << this->m_dwOptions;	//Options Mask 0x0024E568L; <-stretch UI Normal -> 0x0000E568L;

	//k109:  There are 7 spell tabs total

	for(i = 0; i < 7; i++)
	{
		
		if(m_SpellTabs[i].dwTabCount == 0)
		{
			cmRet << 0L;
		}
		else
		{
			
		
			cmRet << m_SpellTabs[i].dwTabCount;

			UpdateConsole("Tab: %08x\r\n",m_SpellTabs[i].dwTabCount);

			for(int b = 0; b < int(m_SpellTabs[i].dwTabCount); b++)
			{
				cmRet << m_SpellTabs[i].dwSpell_ID[b];
			}
		}
		
	}
	
	int inv_count = 0;
	int equipCount = 0;
	int unequipCount = 0;

	for ( iterObject_lst itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
	{
		cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
															
		this->m_vInventory[inv_count].dwObjectGUID = ( *itObject )->GetGUID();
		this->m_vInventory[inv_count].dwIsContainer = pcItemInv->m_isContainer;
		this->m_vInventory[inv_count].fEquipped = ( *itObject )->m_fEquipped;
		this->m_vInventory[inv_count].dwItemModelNumber = ( *itObject )->GetItemModelID();

		if ( ( *itObject )->m_fEquipped == 2 )
		{
			equipCount++;
		} else {
			unequipCount++;
		}
		inv_count++;
	}

	/*
	//spell entries;
	cmRet << WORD( 0x23 ) << WORD( 0x40 );
	DWORD unkfloat = 0x3F800000;
	cmRet << 63L   << unkfloat << 0L << 0L;
	cmRet << 2483L << unkfloat << 0L << 0L;
	cmRet << 2416L << unkfloat << 0L << 0L;
	cmRet << 2368L << unkfloat << 0L << 0L;
	cmRet << 2302L << unkfloat << 0L << 0L;
	cmRet << 2366L << unkfloat << 0L << 0L;
	//
	cmRet << 2075L << unkfloat << 0L << 0L; // Imperil Self VII
	cmRet << 2928L << unkfloat << 0L << 0L; // Tusker Hide
	cmRet << 2929L << unkfloat << 0L << 0L; // Tusker Might
	cmRet << 2930L << unkfloat << 0L << 0L; // Tusker Skin
	cmRet << 2932L << unkfloat << 0L << 0L; // Tusker Leap
	cmRet << 2933L << unkfloat << 0L << 0L; // Tusker Sprint
	cmRet << 2934L << unkfloat << 0L << 0L; // Tusker Fists
	cmRet << 2791L << unkfloat << 0L << 0L; // Rolling Death
	cmRet << 3370L << unkfloat << 0L << 0L; // Winds of Celid
	cmRet << 1832L << unkfloat << 0L << 0L; // Torrential Acid
	cmRet << 1833L << unkfloat << 0L << 0L; // Squall of Swords
	cmRet << 1834L << unkfloat << 0L << 0L; // Firestorm
	cmRet << 1835L << unkfloat << 0L << 0L; // Splinterfall
	cmRet << 1836L << unkfloat << 0L << 0L; // Avalanche
	cmRet << 1837L << unkfloat << 0L << 0L; // Lighting Barrage
	cmRet << 1838L << unkfloat << 0L << 0L; // Stone Fists
	cmRet << 2048L << unkfloat << 0L << 0L; // Boon of the Demon
	cmRet << 2949L << unkfloat << 0L << 0L; // Bile of the Hopeslayer
	cmRet << 2357L << unkfloat << 0L << 0L; // Fauna Perlustration
	cmRet << 2672L << unkfloat << 0L << 0L; // Ring of True Pain
	cmRet << 2673L << unkfloat << 0L << 0L; // Ring of Unspeakable Agony
	cmRet << 2674L << unkfloat << 0L << 0L; // Vicious Rebuke
	cmRet << 2701L << unkfloat << 0L << 0L; // Elemental Fury
	cmRet << 2702L << unkfloat << 0L << 0L; // Elemental Fury
	cmRet << 2703L << unkfloat << 0L << 0L; // Elemental Fury
	cmRet << 2704L << unkfloat << 0L << 0L; // Elemental Fury
	cmRet << 2705L << unkfloat << 0L << 0L; // Elementalist's Boon
	cmRet << 2710L << unkfloat << 0L << 0L; // Volcanic Blast
	cmRet << 2941L << unkfloat << 0L << 0L; // Ulgrim's Recall

	cmRet << 0x4L;	//Login Mask 3
	cmRet << 0x0100E568L;	//Options Mask 0x0024E568L; <-stretch UI Normal -> 0x0000E568L; PLay sound only when active window 0x0100E568L
	cmRet << 0x6L << 2483L << 2416L << 2368L << 2302L << 2366L << 63L;; // Spell Tab 1
	cmRet << 0L << 0L << 0L << 0L; // Spell Tab2,3,4,5
*/	
	cmRet << unequipCount;	// inventory count

	for( int j = 0; j < unequipCount; j++)
	{
		if (this->m_vInventory[j].fEquipped != 2)
		{
			cmRet << this->m_vInventory[j].dwObjectGUID << this->m_vInventory[j].dwIsContainer;
		}
	}

	//Vector of Inventory

	//Item ID followed by container information
//	cmRet << 0xEB5797D6 << 0x0L; // Amuli Coat
//	cmRet << 0x81A02865 << 0x0L; // Pyreals
//		cmRet << 0L;

	cmRet << equipCount; // equipment count
	// Vector of Equipment Count
	for( int k = 0; k < equipCount; k++)
	{
		if (this->m_vInventory[k].fEquipped == 2)
		{
			cItemModels *pcItemEquip = cItemModels::FindModel( this->m_vInventory[k].dwItemModelNumber );
			cmRet << this->m_vInventory[k].dwObjectGUID << pcItemEquip->m_dwEquipPossible << pcItemEquip->m_dwCoverage;							
		}
	}

	return cmRet;
}
/**
 *	Handles the message sent for the creation of an avatar.
 *
 *	The message includes information on the avatar's model (palette, texture, and model information),
 *	name, icon, PK flag, inventory slots, pack slots, and total pyreals value.
 *
 *	@return cMessage - Returns a Create Object (0x0000F745) server message.
 */
cMessage cAvatar::CreatePacket( )
{
	cMessage cmReturn;
	
	float	flpScale;  // Scale used in Packet being Sent
//	float	flmScale; // Model Data Scale

	struct PaletteChange
	{
		WORD m_wNewPalette;
		BYTE m_ucOffset;
		BYTE m_ucLength;
	} pc;

	WORD wModelID;
	WORD wIconID;
	DWORD dwAnimationStrip = 0x0L;	// Always 0x09000001; portal.dat entry for human animation series
	DWORD dwUnkDatEntry = 0x0L;		// Always 0x20000001; portal.dat entry for sound strings
	DWORD dwUnkDatEntry2 = 0x0L;
	DWORD dwModel = 0x0L;			// The Model always 1 for Human. 0x02 = ?, 00 = ?, 00 = invisible parts, 00 body parts

	cmReturn << 0xF745L << m_dwGUID << BYTE(0x11); //0x11 is a constant
	
	// Model Definition Data Starts


	//switch ( m_bAccessLevel)

	if (m_wModelNum != 0) 
	{
		char	szCommand[200];
	//	char	ModelName[75];
		WORD	wModelNumber;
		WORD	PaletteVector;
		WORD	TextureVector;
		WORD	ModelVector;
		BYTE	bPaletteChange;
		BYTE	bTextureChange;
		BYTE	bModelChange;
	//	BYTE	bModelIndex;
	//	WORD	wOldTexture;
	//	WORD	wNewTexture;
	//	WORD	wNewModel;
		WORD	wPaletteCode;

		RETCODE	retcode;
		unsigned char ModelArray[255] = {0x00,};
		int iCol = 0;
		char	readbuff01[5];
		char	readbuff02[5];
		char	readbuff03[9];
		char	readbuff04[9];
		char	readbuff05[9];
		char	readbuff06[9];
		char	readbuff07[5];

		// Model Load 
		sprintf( szCommand, "SELECT * FROM avatar_templates WHERE ModelNum=%d;", m_wModelNum);
		
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		
		iCol = 2;
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &wModelNumber, sizeof( wModelNumber ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &PaletteVector, sizeof( PaletteVector ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bPaletteChange, sizeof( BYTE ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &TextureVector, sizeof( TextureVector ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bTextureChange, sizeof( BYTE ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_ULONG, &ModelVector, sizeof( ModelVector ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bModelChange, sizeof( BYTE ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01, sizeof( readbuff01 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02, sizeof( readbuff02 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03, sizeof( readbuff03 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff04, sizeof( readbuff04 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff05, sizeof( readbuff05 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff06, sizeof( readbuff06 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff07, sizeof( readbuff07 ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
		retcode = SQLFetch( cDatabase::m_hStmt ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		
		//Scan the text into hexadecimal
		sscanf(readbuff01,"%08x",&wModelID);
		sscanf(readbuff02,"%08x",&wIconID);
		sscanf(readbuff03,"%08x",&dwAnimationStrip);
		sscanf(readbuff04,"%08x",&dwUnkDatEntry);
		sscanf(readbuff05,"%08x",&dwUnkDatEntry2);
		sscanf(readbuff06,"%08x",&dwModel);
		sscanf(readbuff07,"%08x",&wPaletteCode);
		
		m_bBasicPaletteChange = m_wBasicPaletteVector;
		m_bBasicTextureChange = m_wBasicTextureVector;
		m_bBasicModelChange	 = m_wBasicModelVector;

		//Correct value fomatting
		dwAnimationStrip += 0x09000000;
		dwUnkDatEntry += 0x20000000;
		dwUnkDatEntry2 += 0x34000000;
		dwModel += 0x02000000;

		// Add the vector counts to the packet
		//cmReturn << bPaletteChange << bTextureChange << bModelChange;
		//cmReturn << m_bBasicPaletteChange << m_bBasicTextureChange << m_bBasicModelChange;

		int paletteChange = m_bBasicPaletteChange;
		int textureChange = m_bBasicTextureChange;

		//Loop through the palettes and textures of currently equipped items
		//Check whether they contribute palettes and textures to the character model
		for ( iterObject_lst itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
		{
			if ( ( *itObject )->m_fEquipped == 2 )
			{
				cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
				if (pcItemInv->m_PortalLinker != 0)
				{
					if (this->m_wGender == 0)
						cPortalDat::LoadItemModel(( *itObject ), 0x0200004E);
					else
						cPortalDat::LoadItemModel(( *itObject ), 0x02000001);
				}

				if ( ( *itObject )->m_bWearPaletteChange != 0)
					paletteChange += ( *itObject )->m_bWearPaletteChange;
				if ( pcItemInv->m_bWearTextureChange != 0)
					textureChange += pcItemInv->m_bWearTextureChange;
			}
		}

		cmReturn	<< BYTE(paletteChange)
					<< BYTE(textureChange)
					<< BYTE(m_bBasicModelChange);

		// The avatar character model information (which consists of palettes, textures, and models)
		// is built upon the avatar's default appearance and any apparel currently worn. The apparel's
		// palette and texture information is appended to the character's default palette and texture
		// information. The apparel's models supercede/replace the respective character models.

		cmReturn	<< WORD(0x007E);	// the human palette

		//Loop through the avatar's default palettes
		if ( m_bBasicPaletteChange != 0)
		{
			for (int i = 0; i < m_bBasicPaletteChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&m_BasicVectorPal[i],sizeof(m_BasicVectorPal[i]));
			}
		}

		//Loop through the palettes of currently equipped items
		for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
		{
			cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
			if ( ( *itObject )->m_fEquipped == 2 )
			{

				if ( ( *itObject )->m_bWearPaletteChange != 0)
				{
					for (int i = 0; i < ( *itObject )->m_bWearPaletteChange; i++)
					{
						cmReturn.pasteData((UCHAR*)&( *itObject )->m_WearVectorPal[i],sizeof(( *itObject )->m_WearVectorPal[i]));
					}
				}
			}
		}

		//Loop through the avatar's default textures
		if ( m_bBasicTextureChange != 0)
		{
			for (int i = 0; i < m_bBasicTextureChange; i++)
			{
				cmReturn.pasteData((UCHAR*)&m_BasicVectorTex[i],sizeof(m_BasicVectorTex[i]));
			}
		}
		//Loop through the textures of currently equipped items
		for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
		{
			if ( ( *itObject )->m_fEquipped == 2 )
			{
				cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
				if ( pcItemInv->m_bWearTextureChange != 0)
				{
					for (int i = 0; i < pcItemInv->m_bWearTextureChange; i++)
					{
							cmReturn.pasteData((UCHAR*)&pcItemInv->m_WearVectorTex[i],sizeof(pcItemInv->m_WearVectorTex[i]));
					}
				}
			}
		}

		bool modelIsCovered;
		if ( m_bBasicModelChange != 0) 
		{
			//Loop through the avatar's default models
			//Do not include avatar models superceded by item models
			for (int i = 0; i < m_bBasicModelChange; i++)
			{
				modelIsCovered = false;
				//Loop through the models of currently equipped items
				for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
				{
					if ( ( *itObject )->m_fEquipped == 2 )
					{
						cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
						if ( pcItemInv->m_bWearModelChange != 0)
						{
							for (int j = 0; j < pcItemInv->m_bWearModelChange; j++)
							{
								//Find whether the item affects the given avatar body part
								//If so, the item model index will equal the given avatar model index
								if(m_BasicVectorMod[i].m_bModelIndex == pcItemInv->m_WearVectorMod[j].m_bModelIndex)
								{
									modelIsCovered = true;
								}
							}
						}
					}
				}
				if (!modelIsCovered)
					cmReturn.pasteData((UCHAR*)&m_BasicVectorMod[i],sizeof(m_BasicVectorMod[i]));
			}
		}
		//Loop through the models of currently equipped items
		//If two items cover the same area, the one with a higher coverage value supercedes
		for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
		{
			if ( ( *itObject )->m_fEquipped == 2 )
			{
				cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
				if ( pcItemInv->m_bWearModelChange != 0)
				{
					for (int i = 0; i < pcItemInv->m_bWearModelChange; i++)
					{
						modelIsCovered = false;
						for ( iterObject_lst itObject2 = m_lstInventory.begin( ); itObject2 != m_lstInventory.end( ); ++itObject2 )
						{
							if ( ( *itObject2 )->m_fEquipped == 2 )
							{
								cItemModels *pcItemInv2 = cItemModels::FindModel(( *itObject2 )->GetItemModelID());
								if ( pcItemInv2->m_bWearModelChange != 0)
								{
									for (int j = 0; j < pcItemInv2->m_bWearModelChange; j++)
									{
										if (pcItemInv->m_WearVectorMod[i].m_bModelIndex == pcItemInv2->m_WearVectorMod[j].m_bModelIndex)
											if (pcItemInv->m_dwCoverage < pcItemInv2->m_dwCoverage)
												modelIsCovered = true;
									}
								}
							}
						}
						if (!modelIsCovered)
							cmReturn.pasteData((UCHAR*)&pcItemInv->m_WearVectorMod[i],sizeof(pcItemInv->m_WearVectorMod[i]));
					}
				}
			}
		}
	}
	else
	{
		//If a character has no m_modelNum value, just create a "generic" character

		BYTE ucPaletteChangeCount = 3; //changes depending on the player model
		BYTE ucTextureChangeCount = 4;
		BYTE ucModelChangeCount = 17;
		wModelID = 1;
		wIconID = 0x1036;
		dwAnimationStrip = 0x09000001;
		dwUnkDatEntry = 0x20000001;
		dwUnkDatEntry2 = 0x34000004;
		dwModel = 0x02000001;

		cmReturn << ucPaletteChangeCount << ucTextureChangeCount << ucModelChangeCount;
		
		//Make 3 palette changes
		pc.m_wNewPalette = 0x007E;
		pc.m_ucOffset = 174;		// 182 Female
		pc.m_ucLength = 4;			//   2 Female
		cmReturn.pasteData((UCHAR*)&pc,sizeof(pc));

		pc.m_wNewPalette = 0x1800;
		pc.m_ucOffset = 227;		//	0 Female
		pc.m_ucLength = 2;			//	3 Female
		cmReturn.pasteData((UCHAR*)&pc,sizeof(pc));
		
		pc.m_wNewPalette = 0x0818;
		pc.m_ucOffset = 175;		//	191 Female
		pc.m_ucLength = 4;			//	2	Female
		cmReturn.pasteData((UCHAR*)&pc,sizeof(pc));
		
		cmReturn << WORD(0x0820);	//this value is unknown..it only exists if there are palette changes

		//Texture changes control the different textures on a character sub-model. 
		//If the character is naked then there are only 4 (hair, eyes, nose, and mouth).
		//and they all take place on the head sub model.
		//If the character is clothed or has armor, there could be many texture
		//changes.
		
		#pragma pack( push, 1 )
		struct TextureChange
		{
			BYTE m_bModelIndex;
			WORD m_wOldTexture;
			WORD m_wNewTexture;
		} tc;
		#pragma pack(pop)
		
		//Hair Texture Array
		WORD wHairTextures[2][4] = { { 0x10B8, 0x10B8, 0x10B8, 0x10B7 }, { 0x11FD, 0x11FD, 0x11FD, 0x10B7 } };

		//Hair
		tc.m_bModelIndex = 0x10;
		tc.m_wOldTexture = 0x98;
		tc.m_wNewTexture = wHairTextures[m_wGender][m_wHairStyle];
		cmReturn.pasteData((UCHAR*)&tc,sizeof(tc));

		//Forehead
		tc.m_bModelIndex = 0x10;
		tc.m_wOldTexture = 0x24C;

		if( m_wHairStyle == 3 )	// Bald
			tc.m_wNewTexture = g_wAvatarTexturesBaldList[m_wRace][m_wGender][m_wHead];//0x110E;
		else
			tc.m_wNewTexture = g_wAvatarTexturesList[m_wRace][m_wGender][0][m_wHead];

		cmReturn.pasteData((UCHAR*)&tc,sizeof(tc));
		
		//Nose
		tc.m_bModelIndex = 0x10;
		tc.m_wOldTexture = 0x2F5;
		tc.m_wNewTexture = g_wAvatarTexturesList[m_wRace][m_wGender][1][m_wNose];//0x117B;
		cmReturn.pasteData((UCHAR*)&tc,sizeof(tc));
		
		//Chin
		tc.m_bModelIndex = 0x10;
		tc.m_wOldTexture = 0x25C;
		tc.m_wNewTexture = g_wAvatarTexturesList[m_wRace][m_wGender][2][m_wChin];//0x119A;
		cmReturn.pasteData((UCHAR*)&tc,sizeof(tc));
		
		// Female
		BYTE bModelIndexes[] = { 0x00, 0x01, 0x02, 0x05, 0x06, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x03, 0x07, 0x04, 0x08 };
		
		//							Female	Male
		// 0x005A 	- Regular		0x10B8	0x11FD
		// 0x04A7 	- Bulky Hair	0x10B8	0x11FD
		// 0x049E 	- Ponytail		0x10B8	0x11FD
		// 0x005A 	- Baldness		0x10B7	0x10B7

		WORD wNewModels[2][16] = 
		{ 
			{ 0x0477, 0x04BE, 0x04C4, 0x04C6, 0x04C5, 0x04B6, 0x04CF, 0x04CD, 0x0076, 0x04D0, 0x04CE, 0x0077, 0x0479, 0x0478, 0x04BA, 0x04BC }, 
			{ 0x004E, 0x004F, 0x004D, 0x0053, 0x0051, 0x0054, 0x0497, 0x0495, 0x0076, 0x04AD, 0x0496, 0x0077, 0x004C, 0x0050, 0x004B, 0x0052 } 
		};

		WORD wHairModels[4] = { 0x005A, 0x04A7, 0x049E, 0x005A };
		
		for( int i = 0; i < sizeof( bModelIndexes ); i++ )
			cmReturn << bModelIndexes[i] << wNewModels[m_wGender][i];

		// Do Hair Model index is 0x10
		cmReturn << BYTE(0x10) << wHairModels[m_wHairStyle];
	}

	cmReturn.pasteAlign(4);
	
	// End of Model Definition Data

	DWORD dwFlags1 = 0x00019883;
	cmReturn << dwFlags1;

	cmReturn << WORD(0x4410);	// Type of portal user is exiting. For begin it's 0x4410, for end it's 0x0408 
	cmReturn << WORD(0x0040);	// Unknown
	
	//Flags1 Mask
	{
		//Flags1 & 0x0010000 Initial Animation Bytes 
		{
			DWORD dwNumInitAnimationBytes = 8;
			cmReturn << dwNumInitAnimationBytes;
			
			BYTE ucInitialAnimation[] =  //determines in which animation the model starts
			{0x00, 0x00, 0x3D, 0x00, 0x00, 0xF0, 0xD0, 0x03};
			
			for(int blah = 0;blah<sizeof(ucInitialAnimation);blah++)
				cmReturn << ucInitialAnimation[blah];
			
			cmReturn << 0x00000000L;	// Unknown DWORD
		}

		//Flags1 & 0x00008000 Location 
		{
			cmReturn.pasteData( (UCHAR*)&m_Location, sizeof(m_Location) );
		}		//Flags1 & 0x00000002
		{
			//DWORD dwAnimationStrip = 0x09000001; //Always 0x09000001, which is the human animation series
			cmReturn << dwAnimationStrip;
		}


		//Flags1 & 0x00000800
		{
			//DWORD dwUnkDatEntry = 0x20000001; //Always 0x20000001 for the portal.dat entry
			cmReturn << dwUnkDatEntry;		  // sound string
		}
		
		//Flags1 & 0x00001000
		{
			//DWORD dwUnkDatEntry2 = 0x34000004;
			cmReturn << dwUnkDatEntry2;
		}

		//Flags1 & 0x00000001
		{
			//DWORD dwModel = 0x02000001; //the model, always 1 for a human
			cmReturn << dwModel;		// 0x02 = ?, 00 = ?, 00 = invisible parts, 00 body parts
		}
	}
	if (m_flAScale > 0)
	{
		flpScale = m_flAScale;
	}
	else
	{
		if ( m_wModelNum > 0 )
		{
			flpScale = 1;	//flmScale;
		}
		else
		{
			flpScale = 1;
		}
	}

	cmReturn << flpScale; // Avatar Scale

	//Next comes some unknown flags...these flags contain information such
	//as whether or not the object is solid (collision detection), the radar color
	//PK information, and more.  I will crack these some time

	// SeaGreens
	WORD wUnkFlag2 = 0x0001;
	WORD wUnkFlag3 = 0x0001;
	WORD wUnkFlag4 = 0;
	WORD wUnkFlag6 = 0;
	WORD wUnkFlag7 = 0;
	WORD wUnkFlag8 = 0;
	WORD wUnkFlag10 = 0;

	cmReturn	<< m_wPositionSequence	//movement
				<< wUnkFlag2			//animations
				<< wUnkFlag3			//bubble modes
				<< wUnkFlag4			//num jumps
				<< m_wNumPortals 
				<< wUnkFlag6			//anim count
				<< wUnkFlag7			//overrides
				<< wUnkFlag8
				<< m_wNumLogins
				<< wUnkFlag10;
	
	DWORD dwFlags2 = 0x00800016;		//Flags2 Defines what data comes next
	
	if (m_dwAllegianceID != 0)
		dwFlags2 = dwFlags2 & 0x00800056;	// WORD 0040 - Monarch Information

	cmReturn << dwFlags2;				// Word 0080 - Flag Word 0016 Length of Name
	
	cmReturn << Name( );				// Object's Name

	cmReturn << wModelID << wIconID;

	DWORD dwObjectFlags1 = 0x00000010;	//Player Object Flags1, 0x00000010 = NPK Player/Monster
	DWORD dwObjectFlags2 = 0x0000001C;	//Player Object Flags2, 0x00000010 = Selectable, 0x00000004 = Cannot be Picked up, 0x00000008 = Player
	
	if( m_fIsPK == 1 )
		dwObjectFlags2 |= 0x00000020;	// PK Flag
	if( m_fIsPK == 2 )
		dwObjectFlags2 |= 0x02000000;	// PKLite Flag
	if( m_fIsPK == 3 )
		dwObjectFlags2 = 0x00000014;	// 0014 - Monster Flag Orange Dot
	if( m_fIsPK == 4 )
		dwObjectFlags2 = 0x00100012;	// 0010 - blue dot
	if( m_lstInventory.size() > 0 )
		dwObjectFlags2 |= 0x00010000; 

	cmReturn << dwObjectFlags1 << dwObjectFlags2;
	

	//Flags2 Mask
	{
		//Flags2 & 0x00000002
		if (dwFlags2 & 0x00000002)
		{
			BYTE bNumberOfSlots = 102;	//number of slots in the main inventory pack
			cmReturn << bNumberOfSlots;
		}

		//Flags2 & 0x00000004
		if (dwFlags2 & 0x00000004)
		{	
			BYTE bNumPackSlots = 7;		//number of packs that can be stored
			cmReturn << bNumPackSlots;
		}

		//Flags2 & 0x00000010
		if (dwFlags2 & 0x00000010)
		{
			WORD wTotalStackValue = 5;	// Total Value of Stack Items
			cmReturn << wTotalStackValue;
		}

		//Flags2 & 0x00000040
		if (dwFlags2 & 0x00000040)
		{
			if (cAllegiance* aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID))
			{
				cmReturn << DWORD(aAllegiance->GetLeader());	// Monarch GUID
			} else {
				cmReturn << 0x00000000;
			}
		}

		//Flags2 & 0x00800000
		if (dwFlags2 & 0x00800000)
		{
			DWORD dwUnknown4 = 0x00040000;
			cmReturn << dwUnknown4;
		}
	}

	//UpdateConsole("Avatar model loaded.\r\n");	
	return cmReturn;
}

cMessage cAvatar::UpdateAvatarModel( )
{
	cMessage cWrChange;
	//cItemModels *pcModel = cItemModels::FindModel(pcObj->GetItemModelID());	
	//pcModel->m_dwEquipActual = 0L;
	//pcModel->m_dwFlags1 = (pcModel->m_dwFlags1 ^ 0x00020000);

	int paletteChange = m_bBasicPaletteChange;
	int textureChange = m_bBasicTextureChange;
	//Loop through the palettes and textures of previously equipped items
	for ( iterObject_lst itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
	{
		if ( ( *itObject )->m_fEquipped == 2 )
		{
			cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
			if ( ( *itObject )->m_bWearPaletteChange != 0)
				paletteChange += ( *itObject )->m_bWearPaletteChange;
			if ( pcItemInv->m_bWearTextureChange != 0)
				textureChange += pcItemInv->m_bWearTextureChange;
		}
	}

	cWrChange	<< DWORD(0xF625) 	
				<< DWORD(m_dwGUID)
				<< BYTE(0x11)  // 11 Vector Palettes
				<< BYTE(paletteChange)
				<< BYTE(textureChange)
				<< BYTE(m_bBasicModelChange);

	cWrChange << WORD(0x007E);
		
	//Loop through the avatar's default palettes
	if ( m_bBasicPaletteChange != 0)
	{
		for (int i = 0; i < m_bBasicPaletteChange; i++)
		{
			cWrChange.pasteData((UCHAR*)&m_BasicVectorPal[i],sizeof(m_BasicVectorPal[i]));
		}
	}
	//Loop through the palettes of previously equipped items
	for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
	{
		if ( ( *itObject )->m_fEquipped == 2 )
		{
			cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
			if ( ( *itObject )->m_bWearPaletteChange != 0)
			{
				for (int i = 0; i < ( *itObject )->m_bWearPaletteChange; i++)
				{
					cWrChange.pasteData((UCHAR*)&( *itObject )->m_WearVectorPal[i],sizeof(( *itObject )->m_WearVectorPal[i]));
				}
			}
		}
	}

	//Loop through the avatar's default textures
	if ( m_bBasicTextureChange != 0)
	{
		for (int i = 0; i < m_bBasicTextureChange; i++)
		{
			cWrChange.pasteData((UCHAR*)&m_BasicVectorTex[i],sizeof(m_BasicVectorTex[i]));
		}
	}
	//Loop through the textures of previously equipped items
	for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
	{
		if ( ( *itObject )->m_fEquipped == 2 )
		{
			cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
			if ( pcItemInv->m_bWearTextureChange != 0)
			{
				for (int i = 0; i < pcItemInv->m_bWearTextureChange; i++)
				{
					cWrChange.pasteData((UCHAR*)&pcItemInv->m_WearVectorTex[i],sizeof(pcItemInv->m_WearVectorTex[i]));
				}
			}
		}
	}

	bool modelIsCovered;
	if ( m_bBasicModelChange != 0) 
	{
		//Loop through the avatar's default models
		//Do not include avatar models superceded by item models
		for (int i = 0; i < m_bBasicModelChange; i++)
		{
			modelIsCovered = false;

			//Loop through the models of previously equipped items
			for ( itObject =m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
			{
				if ( ( *itObject )->m_fEquipped == 2 )
				{
					cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
					if ( pcItemInv->m_bWearModelChange != 0)
					{
						for (int j = 0; j < pcItemInv->m_bWearModelChange; j++)
						{
							//Find whether the item affects the given avatar body part
							//If so, the item model index will equal the given avatar model index
							if(m_BasicVectorMod[i].m_bModelIndex == pcItemInv->m_WearVectorMod[j].m_bModelIndex)
							{
								modelIsCovered = true;
							}
						}
					}
				}
			}

			if (!modelIsCovered)
				cWrChange.pasteData((UCHAR*)&m_BasicVectorMod[i],sizeof(m_BasicVectorMod[i]));
		}
	}
	//Loop through the models of currently equipped items
	//If two items cover the same area, the one with a higher coverage value supercedes
	for ( itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
	{
		if ( ( *itObject )->m_fEquipped == 2 )
		{
			cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
			if ( pcItemInv->m_bWearModelChange != 0)
			{
				for (int i = 0; i < pcItemInv->m_bWearModelChange; i++)
				{
					modelIsCovered = false;
					for ( iterObject_lst itObject2 = m_lstInventory.begin( ); itObject2 != m_lstInventory.end( ); ++itObject2 )
					{
						if ( ( *itObject2 )->m_fEquipped == 2 )
						{
							cItemModels *pcItemInv2 = cItemModels::FindModel(( *itObject2 )->GetItemModelID());
							if ( pcItemInv2->m_bWearModelChange != 0)
							{
								for (int j = 0; j < pcItemInv2->m_bWearModelChange; j++)
								{
									if (pcItemInv->m_WearVectorMod[i].m_bModelIndex == pcItemInv2->m_WearVectorMod[j].m_bModelIndex)
										if (pcItemInv->m_dwCoverage < pcItemInv2->m_dwCoverage)
											modelIsCovered = true;
								}
							}
						}
					}
					if (!modelIsCovered)
						cWrChange.pasteData((UCHAR*)&pcItemInv->m_WearVectorMod[i],sizeof(pcItemInv->m_WearVectorMod[i]));
				}
			}
		}
	}
											
	cWrChange.pasteAlign(4);
	cWrChange	<< m_wNumLogins;
	cWrChange	<< ++m_wModelSequence;

	return cWrChange;				
}
/**
 *	Handles the message sent for an avatar's housing information.
 *
 *	The message includes information on the house's purchase time, type,purchase and maintaince costs, name, and location.
 *
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type House Information for Owners (0x00000225) or House Information for Non-Owners (0x00000226).
 */
cMessage cAvatar::HousingInfo(DWORD F7B0seq)
{
	char	szCommand[512];
	RETCODE	retcode;

	DWORD	dwAccountID;
	UINT	dwAvatarGUID;
//	WORD	wAvatarCount;
	DWORD	dwHouseType;
	DWORD	dwPurchaseTime;

	int i;
	cMessage cmReturn;
	cmReturn	<< 0xF7B0L
				<< m_dwGUID
				<< F7B0seq;	// sequence number
	
	sprintf( szCommand, "SELECT * FROM avatar WHERE AvatarGUID = %lu;",m_dwGUID );									
	retcode = SQLPrepare( cDatabase::m_hStmt,(unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );

	retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwAccountID, sizeof( dwAccountID ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

	if (SQLFetch( cDatabase::m_hStmt ) != SQL_SUCCESS)
	{
		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
	} else {
		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

//		sprintf( szCommand, "SELECT COUNT(houses_covenants.ID) FROM {oj avatar LEFT OUTER JOIN houses_covenants ON avatar.AvatarGUID=houses_covenants.OwnerID} WHERE avatar.OwnerID = %d;",AccountID );

		bool owner = false;
		dwAvatarGUID = NULL;

		sprintf( szCommand, "SELECT AvatarGUID FROM avatar WHERE OwnerID = %d;",dwAccountID );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwAvatarGUID, sizeof( dwAvatarGUID ), NULL );CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		
		DWORD avatarArray[10];

		for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
		{
			avatarArray[i] = dwAvatarGUID;
		}

		retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
		
		for ( i = 0; owner != true && i < 10; i++ )
		{
			sprintf( szCommand, "SELECT HouseID FROM houses_covenants WHERE OwnerID = %d;",avatarArray[i] );
			retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLExecute( cDatabase::m_hStmt );														CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &m_wHouseID, sizeof( WORD ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

			if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
			{
				owner = true;
			}
			
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	
		}
	
		if( owner != true )
		{
			cmReturn << 0x0226L;
		} else {
			sprintf( szCommand, "SELECT HouseType,PurchaseTime FROM houses_covenants WHERE HouseID = %d;",m_wHouseID );
																
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHouseType, sizeof( dwHouseType ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwPurchaseTime, sizeof( dwPurchaseTime ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFetch( cDatabase::m_hStmt );

			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )			
			cmReturn	<< 0x0225L
						<< dwPurchaseTime
						<< dwPurchaseTime	// maintainence last paid?
						<< dwHouseType		// Cottage = 1; Villa = 2; Mansion = 3; Apartment = 4
						<< DWORD(0x0L);		// unknown (0x00000000)

			DWORD dwPurchaseCount;	// Apartment = 2; Cottage, Villa = 3; Mansion = 6;
			sprintf( szCommand, "SELECT COUNT(ID) FROM houses_purchase WHERE HouseID = %d;",m_wHouseID );
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwPurchaseCount, sizeof( dwPurchaseCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLFetch( cDatabase::m_hStmt );
			if( retcode == SQL_NO_DATA )
				dwPurchaseCount = 0;
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
			cmReturn << dwPurchaseCount;

			DWORD dwBuyType;
			DWORD dwBuyRequired;
			DWORD dwBuyPaid;
//			String buyName;
//			String buyPluralName;

			sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_purchase WHERE HouseID = %d;",m_wHouseID );														
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwBuyType, sizeof( dwBuyType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwBuyRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwBuyPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
			{
				cItemModels *pcModel = cItemModels::FindModel(dwBuyType);

				std::string		strPluralName = pcModel->m_strName.c_str();
  				strPluralName.assign(strPluralName);

				cmReturn	<< dwBuyRequired				// quantity required
							<< dwBuyPaid					// quantity paid
							<< DWORD(pcModel->m_wModel)		// item's object type
							<< pcModel->m_strName.c_str()	// name of this item
							<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
			}
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
			
			DWORD dwMaintenanceCount;		// Apartment, Cottage = 1; Villa, Mansion = 2;
			sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",m_wHouseID );
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLFetch( cDatabase::m_hStmt );
			if( retcode == SQL_NO_DATA )
				dwMaintenanceCount = 0;
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
			cmReturn << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling
			
			DWORD dwMaintainType;
			DWORD dwMaintainRequired;
			DWORD dwMaintainPaid;
//			String maintainName;
//			String maintainPluralName;	
	
			sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",m_wHouseID );														
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintainType, sizeof( dwMaintainType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
			retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwMaintainRequired, sizeof( dwMaintainRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwMaintainPaid, sizeof( dwMaintainPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

			for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
			{
				cItemModels *pcModel = cItemModels::FindModel(dwMaintainType);
				std::string		strPluralName = pcModel->m_strName.c_str();
  				strPluralName.assign(strPluralName);

				cmReturn	<< dwMaintainRequired			// quantity required
							<< dwMaintainPaid				// quantity paid
							<< DWORD(pcModel->m_wModel)		// item's object type
							<< pcModel->m_strName.c_str()	// name of this item
							<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
			}
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

			cLocation	locCovenant;
			char		locBuff[9];
			char		dwCovPosX[9];
			char		dwCovPosY[9];
			char		dwCovPosZ[9];
			char		dwCovOrientW[9];
			char		dwCovOrientX[9];
			char		dwCovOrientY[9];
			char		dwCovOrientZ[9];
			sprintf( szCommand, "SELECT Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z FROM houses_covenants WHERE HouseID = %d;",m_wHouseID );	
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			int iCol = 1;
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, locBuff, sizeof( locBuff ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
/*	 		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flX, sizeof( &locCovenant.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flY, sizeof( &locCovenant.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flZ, sizeof( &locCovenant.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flA, sizeof( &locCovenant.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flB, sizeof( &locCovenant.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flC, sizeof( &locCovenant.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flW, sizeof( &locCovenant.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
*/			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovPosX, sizeof( dwCovPosX ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovPosY, sizeof( dwCovPosY ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovPosZ, sizeof( dwCovPosZ ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientW, sizeof( dwCovOrientW ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientX, sizeof( dwCovOrientX ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientY, sizeof( dwCovOrientY ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, dwCovOrientZ, sizeof( dwCovOrientZ ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
			{
				sscanf(locBuff,"%08x",&locCovenant.m_dwLandBlock);
				sscanf(dwCovPosX,"%08x",&locCovenant.m_flX);
				sscanf(dwCovPosY,"%08x",&locCovenant.m_flY);
				sscanf(dwCovPosZ,"%08x",&locCovenant.m_flZ);
				sscanf(dwCovOrientW,"%08x",&locCovenant.m_flA);
				sscanf(dwCovOrientX,"%08x",&locCovenant.m_flB);
				sscanf(dwCovOrientY,"%08x",&locCovenant.m_flC);
				sscanf(dwCovOrientZ,"%08x",&locCovenant.m_flW);
			}
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
			cmReturn << locCovenant;
		}
	}
	
	return cmReturn;
}

int cAvatar::CastMain(LPVOID wp, LPVOID lp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	//char szMessage[100];
	//sprintf( szMessage, "dwAnim: %u\r\n",basic);
	//UpdateConsole((char *)szMessage);
	//cMessage cPart = pcClient->m_pcAvatar->Particle( 0x50 );
	//cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, cPart, 3 );
	return -1;
}

int cAvatar::WarAnimation1(LPVOID wp, LPVOID lp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cClient* pcClient = cClient::FindClient( WarJobParam->GetCasterGUID() );
	cSpell* pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	
	cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, pcClient->m_pcAvatar->LocationPacket(), 3 );
	
	cLocation CastLocation = WarJobParam->GetCastLocation();
	//if ( CastLocation.m_flA == pcClient->m_pcAvatar->m_Location.m_flA )

	cMessage cmSpellWords = pcSpell->GetCastWords();

	cmSpellWords << pcClient->m_pcAvatar->Name( );
	//cmSpellWords.pasteAlign(2);
	cmSpellWords << pcClient->m_pcAvatar->m_dwGUID << 0x11L; 

	pcClient->m_pcAvatar->SetLocation(pcClient->m_pcAvatar->m_Location);

	cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, cmSpellWords, 4 );

	if (pcSpell->m_dwLevel != 1)
	{
		cMessage msg1 = pcClient->m_pcAvatar->WindupAnimation( pcSpell->GetWindup(), 1.0f );
		cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, msg1, 3 );
	}

	int iDelay = pcSpell->GetWindupDelay();

	int iJob = cMasterServer::m_pcJobPool->CreateJob( &cAvatar::WarAnimation2, (LPVOID) WarJobParam, NULL, "WarAnimation2", iDelay, 1);

	return -1;
}

int cAvatar::WarAnimation2(LPVOID wp, LPVOID lp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cClient* pcClient = cClient::FindClient( WarJobParam->GetCasterGUID() );
	if (!pcClient)
	{
		/*cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
		pcClient->AddPacket( WORLD_SERVER,cmActionComplete,4);
		*/
		SAFEDELETE( WarJobParam )
		return 2;
	}

	cSpell *pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	if (!pcSpell)
	{
		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
		pcClient->AddPacket( WORLD_SERVER,cmActionComplete,4);
		pcClient->m_pcAvatar->m_fIsCasting = false;

		SAFEDELETE( WarJobParam )
		return 2;
	}

	BYTE bAnim = pcSpell->GetCastAnim();

	cMessage msgTest = pcClient->m_pcAvatar->WarAnimation( bAnim, 1.0f );
	cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, msgTest, 3 );

	int iDelay = pcSpell->GetCastAnimDelay()/4 + 1;

	int iJob = cMasterServer::m_pcJobPool->CreateJob( &cAvatar::WarAnimation3, (LPVOID) WarJobParam, NULL, "WarAnimation3", iDelay, 1);

	return -1;
}

int cAvatar::WarAnimation3(LPVOID wp, LPVOID lp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cClient* pcClient = cClient::FindClient( WarJobParam->GetCasterGUID() );
	if (!pcClient)
	{
		/*cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
		pcClient->AddPacket( WORLD_SERVER,cmActionComplete,4);
		*/
		SAFEDELETE( WarJobParam )
		return 2;
	}

	cSpell *pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	if (!pcSpell)
	{
		cMessage cmActionComplete;
		cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
		pcClient->AddPacket( WORLD_SERVER,cmActionComplete,4);
		pcClient->m_pcAvatar->m_fIsCasting = false;

		SAFEDELETE( WarJobParam )
		return 2;
	}
	cMessage msgTest2 = pcClient->m_pcAvatar->WarAnimation2( 1.0f );
	cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, msgTest2, 3 );
	
	float flRange = cPhysics::GetRange(pcClient->m_pcAvatar->m_Location, WarJobParam->GetCastLocation());

	if ( flRange > .15 )
	{
		cMessage cPart = pcClient->m_pcAvatar->Particle( 0x50 );
		cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, cPart, 3 );
		cMessage cmFizzle;
		cmFizzle << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x028AL << 0x0402L;
		pcClient->AddPacket( WORLD_SERVER,cmFizzle,4);
	}
	else if ( strcmp(pcSpell->m_strSchool.c_str(), "War" ) == 0 )
	{
		cAvatar::CastWar( (LPVOID) WarJobParam );
	}
	else if ( strcmp( pcSpell->m_strSchool.c_str(), "Life" ) == 0 )
	{
		cAvatar::CastLife( (LPVOID) WarJobParam );
	}
	else if ( strcmp( pcSpell->m_strSchool.c_str(), "Creature" ) == 0 )
	{
		cAvatar::CastCreature( (LPVOID) WarJobParam );
	}
	else if ( strcmp( pcSpell->m_strSchool.c_str(), "Item" ) == 0 )
	{
		cAvatar::CastItem( (LPVOID) WarJobParam );
	}

	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcClient->m_pcAvatar->GetGUID( ) << ++pcClient->m_dwF7B0Sequence << 0x01C7L << 0L;
	pcClient->AddPacket( WORLD_SERVER,cmActionComplete,4);
	pcClient->m_pcAvatar->m_fIsCasting = false;

	SAFEDELETE( WarJobParam )

	return -1;
}

/**
 *	Handles the message sent for the casting of a War Magic spell.
 *
 *	This function is called whenever a War Magic spell is cast by a client's avatar.
 *	Returns a server message to the client.
 */
void cAvatar::CastWar( LPVOID wp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cSpell *pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	if (pcSpell)
	{
		switch ( pcSpell->m_dwEffect )
		{
		case 117: case 118: case 119: case 120: case 121: case 122: case 123: //bolt, arc
			{
				int iWarModel = SHOCK_MODEL;
				switch ( pcSpell->m_dwEffect )
				{
				case 117: //acid
					{
						iWarModel = ACID_MODEL;
						break;
					}
				case 118: //shock
					{
						iWarModel = SHOCK_MODEL;
						break;
					}
				case 119: //frost
					{
						iWarModel = FROST_MODEL;
						break;
					}
				case 120: //lightning
					{
						iWarModel = LIGHTNING_MODEL;
						break;
					}
				case 121: //flame
					{
						iWarModel = FLAME_MODEL;
						break;
					}
				case 122: //force
					{
						iWarModel = FORCE_MODEL;
						break;
					}
				case 123: //blade
					{
						iWarModel = BLADE_MODEL;
						break;
					}
				}

				cClient* pcClient = cClient::FindClient( WarJobParam->GetCasterGUID() );
				if (pcClient)
				{
					float flUserHeading = cPhysics::GetAvatarHeading( pcClient->m_pcAvatar->m_Location );
					float flTargetHeading;
					cVelocity tarVel;
					
					cClient* pcTargetObj = cClient::FindClient( WarJobParam->GetTargetGUID() );
					
					if( !pcTargetObj )
					{
						cObject* pcTargetObject = cWorldManager::FindObject( WarJobParam->GetTargetGUID() );
						if( !pcTargetObject )
						{
						}
						else
						{
							flTargetHeading = cPhysics::GetHeadingTarget(pcClient->m_pcAvatar->m_Location,pcTargetObject->m_Location);
							tarVel = cPhysics::GetTargetVelocity(pcClient->m_pcAvatar->m_Location,pcTargetObject->m_Location);
							char szMessage[100];
							sprintf( szMessage, "usrX: %f, usrY: %f, tarX: %f, tarY: %f\r\n", pcClient->m_pcAvatar->m_Location.m_flX, pcClient->m_pcAvatar->m_Location.m_flY, pcTargetObject->m_Location.m_flX, pcTargetObject->m_Location.m_flY);
							//UpdateConsole((char *)szMessage);
						}
					}
					else
					{
						flTargetHeading = cPhysics::GetHeadingTarget(pcClient->m_pcAvatar->m_Location,pcTargetObj->m_pcAvatar->m_Location);
						tarVel = cPhysics::GetTargetVelocity(pcClient->m_pcAvatar->m_Location,pcTargetObj->m_pcAvatar->m_Location);
					}
					
					cWarSpell* warSpell = new cWarSpell(cWorldManager::NewGUID_Object(), WarJobParam->GetSpellID(), pcClient->m_pcAvatar->m_Location, tarVel, iWarModel);
										
					cWorldManager::AddObject( warSpell, TRUE );

					cMessage msgParticles = warSpell->WarParticle(warSpell,0x0004,1.0f);
					cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, msgParticles, 3 );

					//cMessage msgSpellAnim = warSpell->SpellAnim(warSpell,0x0049L,0x003CL);
					//cWorldManager::SendToAllInFocus( pcClient->m_pcAvatar->m_Location, msgSpellAnim, 3 );
					
					cSpellMoveParam* SpellMoveParam = new cSpellMoveParam( pcClient->m_pcAvatar->GetGUID(), warSpell->m_Location, warSpell->GetGUID() );
					
					int iJob = cMasterServer::m_pcJobPool->CreateJob( warSpell->Move, (LPVOID) SpellMoveParam, NULL, "WarSpellMove", 1, 200);
				}
				break;
			}
		case 131: //acid blast
			{
			}
		case 132: //shock blast
			{
			}
		case 133: //frost blast
			{
			}
		case 134: //lightning blast
			{
			}
		case 135: //flame blast (also some monster spells)
			{
			}
		case 137: //blade blast
			{
			}
		case 207: //acid volley
			{
			}
		case 208: //bludgeoning volley
			{
			}
		case 209: //frost volley
			{
			}
		case 210: //lightning volley
			{
			}
		case 211: //flame volley
			{
			}
		case 212: //force volley
			{
			}
		case 213: //blade volley
			{
			}
		case 222: //searing disc (8 waves of acid outward from caster)
			{
			}
		case 223: //tectonic rifts (8 shock waves outward from caster), other similar shock wave spells
			{
			}
		case 224: //halo of frost (8 waves of frost outward from caster)
			{
			}
		case 225: //eye of the storm (8 waves of lightning outward from caster)
			{
			}
		case 226: //cassius' ring of fire (8 waves of fire outward from caster)
			{
			}
		case 227: //nuhmudira's spines (8 waves of force outward from caster)
			{
			}
		case 228: //horizon's blades (8 blades outward from caster)
			{
			}
		case 229: //blistering creeper (wall of 5 acid balls, 2 high)
			{
			}
		case 230: //hammering crawler (wall of 5 shockwaves, 2 high)
			{
			}
		case 231: //foon-ki's glacial flow (wall of 5 frost balls, 2 high)
			{
			}
		case 232: //os' wall (wall of 5 lightning bolts, 2 high)
			{
			}
		case 233: //slithering flames (wall of 5 fire balls, 2 high), demon's tongues (wall of lag?)
			{
			}
		case 234: //spike strafe (wall of 5 force bolts, 2 high)
			{
			}
		case 235: //bed of blades (wall of 5 blades, 2 high)
			{
			}
		case 236: //torrential acid (9 streams of acid down at area around target)
			{
			}
		case 237: //raining boulders and other various spells
			{
			}
		case 238: //avalanche (up to 12 balls of frost down at area around target)
			{
			}
		case 239: //lightning barrage (9 bolts of lightning down at area around target)
			{
			}
		case 240: //firestorm (9 balls of flame down at area around target)
			{
			}
		case 241: //splinterfall (9 force bolts down at area around target)
			{
			}
		case 242: //squall of swords (9 blades down at area around target)
			{
			}
		case 243: //acid streak
			{
			}
		case 244: //shock wave streak
			{
			}
		case 245: //frost streak
			{
			}
		case 246: //lightning streak
			{
			}
		case 247: //flame streak
			{
			}
		case 248: //force streak
			{
			}
		case 249: //blade streak
			{
			}
		}
	}
}

/**
 *	Handles the message sent for the casting of a Life Magic spell.
 *
 *	This function is called whenever a Life Magic spell is cast by a client's avatar.
 *	Returns a server message to the client.
 */
void cAvatar::CastLife( LPVOID wp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cSpell *pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	if (pcSpell)
	{
		cMessage msgSpellEffect;
		msgSpellEffect << 0xF755L << WarJobParam->GetTargetGUID() << pcSpell->m_dwEffectAnim << 0x3F2B4E94L;
		
		cClient* pcTargetObj = cClient::FindClient( WarJobParam->GetTargetGUID() );
		if( !pcTargetObj )
		{
			cObject* pcTargetObject = cWorldManager::FindObject( WarJobParam->GetTargetGUID() );
			if( !pcTargetObject )
			{
			}
			else
			{
				cWorldManager::SendToAllInFocus( pcTargetObject->m_Location, msgSpellEffect, 3 );
			}
		}
		else
		{
			cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, msgSpellEffect, 3 );
		}
		switch ( pcSpell->m_dwEffect )
		{
		case 67: //increase caster's health
			{
				if ( pcTargetObj )
				{
					DWORD dwHeal = pcTargetObj->m_pcAvatar->GetHealValue( pcSpell->m_dwLevel );
					int iNewHealth;
					if (pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[0].m_dwCurrent - pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent < dwHeal)
					{
						dwHeal = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[0].m_dwCurrent - pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent;
					}
					iNewHealth = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent;
					iNewHealth += dwHeal;

					cMessage cmHeal1 = pcTargetObj->m_pcAvatar->UpdateHealth(dwHeal,iNewHealth);
					pcTargetObj->AddPacket( WORLD_SERVER,cmHeal1,4);

					char szHeal[200];
					wsprintf( szHeal, "You heal yourself for %d points with %s!", dwHeal, pcSpell->m_strName.c_str() );
					cMessage cmHeal2;
					cmHeal2 << 0xF62C << szHeal << ColorGreen;
					pcTargetObj->AddPacket( WORLD_SERVER, cmHeal2, 4 );
				}
				break;
			}
		case 79: //increase target's health
			{
			}
		case 80: //decrease health (also Martyr's Hecatomb)
			{
			}
		case 81: //increase stamina (also Mana Boost Self)
			{
				if (pcTargetObj)
				{
					DWORD dwStamina = pcTargetObj->m_pcAvatar->GetStaminaValue( pcSpell->m_dwLevel );
					int iNewStamina;
					if (pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_dwCurrent - pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent < dwStamina)
					{
						dwStamina = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_dwCurrent - pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent;
					}
					iNewStamina = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent;
					iNewStamina += dwStamina;
					
					cMessage cmStamina1 = pcTargetObj->m_pcAvatar->UpdateStamina(dwStamina,iNewStamina);
					pcTargetObj->AddPacket( WORLD_SERVER,cmStamina1,4);
					
					char szStamina[200];
					wsprintf( szStamina, "You restore %d points of stamina with %s!", dwStamina, pcSpell->m_strName.c_str() );
					cMessage cmStamina2;
					cmStamina2 << 0xF62C << szStamina << ColorGreen;
					pcTargetObj->AddPacket( WORLD_SERVER, cmStamina2, 4 );
				}
				break;
			}
		case 82: //decrease stamina (also Martyr's Tenacity)
			{
			}
		case 83: //increase target's mana (also some max mana spells)
			{
			}
		case 84: //decrease mana (also Martyr's Blight)
			{
			}
		case 86: //decrease max mana (Malediction)
			{
			}
		case 87: //drain health other, health to stamina, health to mana
			{
			}
		case 88: //infuse health other
			{
			}
		case 89: //drain stamina other, stamina to health, stamina to mana
			{
				if (pcTargetObj)
				{
					float flTransferValue = pcTargetObj->m_pcAvatar->GetTransferValue( pcSpell->m_dwLevel );
					DWORD dwMana = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent / 2 * flTransferValue;
					int iNewMana, iNewStamina;
					
					if (pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[2].m_dwCurrent - pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent < dwMana)
					{
						dwMana = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[2].m_dwCurrent - pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent;
					}
					iNewMana = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent;
					iNewMana += dwMana;
					
					iNewStamina = pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent;
					iNewStamina -= pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent / 2;
					
					cMessage cmMana1 = pcTargetObj->m_pcAvatar->UpdateMana(dwMana,iNewMana);
					pcTargetObj->AddPacket( WORLD_SERVER,cmMana1,4);
					
					cMessage cmStamina = pcTargetObj->m_pcAvatar->DecrementStamina(pcTargetObj->m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent / 2,iNewStamina);
					pcTargetObj->AddPacket( WORLD_SERVER,cmStamina,4);
					
					char szMana[200];
					wsprintf( szMana, "You transfer stamina into mana for %d points with %s!", dwMana, pcSpell->m_strName.c_str() );
					cMessage cmMana2;
					cmMana2 << 0xF62C << szMana << ColorGreen;
					pcTargetObj->AddPacket( WORLD_SERVER, cmMana2, 4 );
				}
				break;
			}
		case 90: //infuse stamina other
			{
			}
		case 91: //drain mana other, mana to health, mana to stamina
			{
			}
		case 92: //infuse mana other
			{
				break;
			}
		case 93: //increase healing rate
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x03L;
				float flValue = pcTargetObj->m_pcAvatar->GetRegenIncValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 94: //decrease healing rate
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x03L;
				float flValue = pcTargetObj->m_pcAvatar->GetRegenDecValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 95: //increase stamina regen
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x04L;
				float flValue = pcTargetObj->m_pcAvatar->GetRegenIncValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 96: //decrease stamina regen
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x04L;	
				float flValue = pcTargetObj->m_pcAvatar->GetRegenDecValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 97: //increase mana rate
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x05L;
				float flValue = pcTargetObj->m_pcAvatar->GetRegenIncValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 98: //decrease mana rate
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x05L;
				float flValue = pcTargetObj->m_pcAvatar->GetRegenDecValue( pcSpell->m_dwLevel );
				
				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 101: //acid prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x45L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 102: //acid vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x45L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 103: //bludgeoning prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x42L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 104: //bludgeoning vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x42L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 105: //cold prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x44L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 106: //cold vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x44L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 107: //lightning prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x46L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 108: //lightning vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x46L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 109: //fire prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x43L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 110: //fire vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x43L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 111: //piercing prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x41L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 112: //piercing vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x41L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 113: //slashing prot
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x40L;
				float flValue = pcTargetObj->m_pcAvatar->GetProtValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 114: //slashing vuln
			{
				DWORD dwFlags		= 0x5008L;
				DWORD dwKey			= 0x40L;
				float flValue = pcTargetObj->m_pcAvatar->GetVulnValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 115: //increase armor
			{
				DWORD dwFlags		= 0xA080L;
				DWORD dwKey			= 0x0L;
				float flValue = pcTargetObj->m_pcAvatar->GetArmorValue( pcSpell->m_dwLevel );

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 116: //decrease armor
			{	
				DWORD dwFlags		= 0xA080L;
				DWORD dwKey			= 0x0L;
				float flValue = pcTargetObj->m_pcAvatar->GetArmorValue( pcSpell->m_dwLevel ) * -1;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 250: //dispels
			{
				break;
			}
		case 273: //increase max health
			{
			}
		case 275: //increase max stamina
			{
			}
		case 277: //increase max mana
			{
			}
		case 279: //increase max health
			{
			}
		case 281: //increase max stamina
			{
			}
		case 283: //increase max mana
			{
			}
		case 285: //acid wards
			{
			}
		case 287: //flame wards
			{
			}
		case 289: //cold wards
			{
			}
		case 291: //lightning wards
			{
			}
		case 295: //timaru's shelter (increase target's armor)
			{
			}
		case 379: //increase target's armor
			{
			}
		case 401: //bludgeon wards
			{
			}
		case 403: //slashing wards
			{
			}
		case 405: //piercing wards
			{
			}
		case 407: //major & minor stamina regen
			{
				DWORD dwFlags		= 0x0L; //0x9001L;
				DWORD dwKey			= 0x0L; //0x01L;
				float flValue		= 0.0f; //pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		}
	}
}

/**
 *	Handles the message sent for the casting of a Creature Enchantment spell.
 *
 *	This function is called whenever a Creature Enchantment spell is cast by a client's avatar.
 *	Returns a server message to the client.
 */
void cAvatar::CastCreature( LPVOID wp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cSpell *pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	if (pcSpell)
	{
		cMessage msgSpellEffect;
		msgSpellEffect << 0xF755L << WarJobParam->GetTargetGUID() << pcSpell->m_dwEffectAnim << 0x3F2B4E94L;
		
		cClient* pcTargetObj = cClient::FindClient( WarJobParam->GetTargetGUID() );
		if( !pcTargetObj )
		{
			cObject* pcTargetObject = cWorldManager::FindObject( WarJobParam->GetTargetGUID() );
			if( !pcTargetObject )
			{
			}
			else
			{
				cWorldManager::SendToAllInFocus( pcTargetObject->m_Location, msgSpellEffect, 3 );
			}
		}
		else
		{
			cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, msgSpellEffect, 3 );
		}
		
		switch ( pcSpell->m_dwEffect )
		{
		case 1: //increase strength
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x01L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 2: //decrease strength
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x01L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 3: //increase endurance
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x02L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 4: //decrease endurance
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x02L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 5: //increase quickness
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x03L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 6: //decrease quickness
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x03L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 7: //increase coordination
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x04L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 8: //decrease coordination
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x04L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 9: //increase focus
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x05L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 10: //decrease focus
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x05L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 11: //increase self
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x06L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 12: //decrease self
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x06L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 13: //increase focus - Concentration 
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x05L;
				float flValue		= 25.0f;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 15: //increase focus - Brilliance
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x05L;
				float flValue		= 50.0f;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 17: //increase axe
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x01L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 18: //decrease axe
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x01L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 19: //increase bow
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x02L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 20: //decrease bow
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x02L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 21: //increase crossbow
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x03L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 22: //decrease crossbow
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x03L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 23: //increase dagger
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x04L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 24: //decrease dagger
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x04L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 25: //increase mace
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x05L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 26: //decrease mace
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x05L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 27: //increase spear
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x09L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 28: //decrease spear
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x09L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 29: //increase staff
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0AL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 30: //decrease staff
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0AL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 31: //increase sword
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0BL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 32: //decrease sword
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0BL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 33: //increase thrown weapons
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0CL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 34: //decrease thrown weapons
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0CL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 35: //increase unarmed combat
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0DL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 36: //decrease unarmed combat
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0DL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 37: //increase melee defense
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x06L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 38: //decrease melee defense
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x06L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 39: //increase missile defense
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x07L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 40: //decrease missile defense
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x07L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 41: //increase magic defense
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0FL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 42: //decrease magic defense
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0FL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 43: //increase creature enchantment
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1FL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 44: //decrease creature enchantment
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1FL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 45: //increase item enchantment
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x20L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 46: //decrease item enchantment
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x20L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 47: //increase life magic
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x21L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 48: //decrease life magic
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x21L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 49: //increase war magic
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x22L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 50: //decrease war magic
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x22L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 51: //increase mana conversion
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x10L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 52: //decrease mana conversion
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x10L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 53: //increase arcane lore
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0EL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 54: //decrease arcane lore
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x0EL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 55: //increase armor tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1DL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 56: //decrease armor tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1DL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 57: //increase item tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x12L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 58: //decrease item tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x12L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 59: //increase magic item tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1EL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 60: //decrease magic item tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1EL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 61: //increase weapon tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1CL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 62: //decrease weapon tinkering
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1CL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 63: //increase assess monster
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1BL;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 64: //decrease assess monster
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x1BL;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 65: //increase deception
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x14L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 66: //decrease deception
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x14L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 67: //increase healing
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x15L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 68: //decrease healing
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x15L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 69: //increase jump
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x16L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 70: //decrease jump
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x16L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 71: //increase leadership
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x23L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 72: //decrease leadership
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x23L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 73: //increase lockpick
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x17L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 74: //decrease lockpick
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x17L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 75: //increase loyalty
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x24L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 76: //decrease loyalty
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x24L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 77: //increase run
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x18L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 78: //decrease run
			{
				DWORD dwFlags		= 0x9001L;
				DWORD dwKey			= 0x18L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 204: //Vitae
			{
				break;
			}
		case 205: //increase assess person
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x13L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 206: //decrease assess person
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x13L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 216: //increase cooking
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x27L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 217: //decrease cooking
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x27L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 218: //increase fletching
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x25L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 219: //decrease fletching
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x25L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 220: //increase alchemy
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x26L;
				float flValue		= pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 221: //decrease alchemy
			{
				DWORD dwFlags		= 0x9010L;
				DWORD dwKey			= 0x26L;
				float flValue		= pcSpell->m_dwLevel * -5 - 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		case 250: //dispels
			{
				break;
			}
		case 251: //increase target's creature enchantment
			{
			}
		case 253: //increase target's item enchantment
			{
			}
		case 255: //increase target's war magic
			{
			}
		case 257: //increase health regen
			{
			}
		case 259: //increase mana regen
			{
			}
		case 261: //increase target's strength
			{
			}
		case 263: //increase target's endurance
			{
			}
		case 265: //increase target's quickness
			{
			}
		case 267: //increase target's coordination
			{
			}
		case 269: //increase target's focus
			{
			}
		case 271: //increase target's self
			{
			}
		case 293: //increase target's leadership
			{
			}
		case 297: //timaru's shelter (increase target's quickness), increase target's missile defense
			{
			}
		case 299: //increase target's magic defense
			{
			}
		case 301: //increase target's coordination
			{
			}
		case 303: //increase target's endurance
			{
			}
		case 305: //increase target's strength
			{
			}
		case 307: //increase target's quickness
			{
			}
		case 309: //increase healing rate
			{
			}
		case 311: //increase target's axe
			{
			}
		case 313: //increase target's dagger
			{
			}
		case 315: //increase target's mace
			{
			}
		case 317: //increase target's spear
			{
			}
		case 319: //increase target's staff
			{
			}
		case 321: //increase target's melee defense
			{
			}
		case 331: //increase target's bow
			{
			}
		case 333: //increase target's alchemy
			{
			}
		case 335: //increase target's arcane lore
			{
			}
		case 337: //increase target's armor tinkering
			{
			}
		case 339: //increase target's cooking
			{
			}
		case 341: //increase target's crossbow
			{
			}
		case 343: //increase target's deception
			{
			}
		case 345: //increase target's loyalty
			{
			}
		case 347: //increase target's fletching
			{
			}
		case 349: //increase target's healing
			{
			}
		case 351: //increase target's melee defense
			{
			}
		case 353: //increase target's item tinkering
			{
			}
		case 355: //increase target's jump
			{
			}
		case 357: //increase target's life magic
			{
			}
		case 359: //increase target's lockpick
			{
			}
		case 361: //increase target's magic item tinkering
			{
			}
		case 363: //increase target's mana conversion
			{
			}
		case 365: //increase target's assess creature
			{
			}
		case 367: //increase target's assess person
			{
			}
		case 369: //increase target's run
			{
			}
		case 371: //increase target's sword
			{
			}
		case 373: //increase target's thrown weapons
			{
			}
		case 375: //increase target's unarmed combat
			{
			}
		case 377: //increase target's weapon tinkering
			{
				DWORD dwFlags		= 0x0L; //0x9010L;
				DWORD dwKey			= 0x0L; //0x01L;
				float flValue		= 0.0f; //pcSpell->m_dwLevel * 5 + 5;

				cEnchantment* pcEnchantment = new cEnchantment( pcSpell->m_dwSpellID, pcSpell->m_dwEffect, pcSpell->m_dwDifficulty, (double) pcSpell->m_iDuration, WarJobParam->GetCasterGUID(), WarJobParam->GetTargetGUID(), (double) timeGetTime(), dwFlags, dwKey, flValue );
				break;
			}
		}
	}
}

/**
 *	Handles the message sent for the casting of an Item Enchantment spell.
 *
 *	This function is called whenever an Item Enchantment spell is cast by a client's avatar.
 *	Returns a server message to the client.
 */
void cAvatar::CastItem( LPVOID wp )
{
	cWarJobParam* WarJobParam = (cWarJobParam *) wp;

	cSpell *pcSpell = cSpell::FindSpell( WarJobParam->GetSpellID() );
	if (pcSpell)
	{
		switch ( pcSpell->m_dwEffect )
		{
		case 152: //increase weapon's attack skill mod
			{
			}
		case 153: //decrease weapon's attack skill mod
			{
			}
		case 154: //increase weapon's damage
			{
			}
		case 155: //decrease weapon's damage
			{
			}
		case 156: //increase weapon's defense skill mod
			{
			}
		case 157: //decrease weapon's defense skill mod
			{
			}
		case 158: //increase weapon's speed
			{
			}
		case 159: //decrease weapon's speed
			{
			}
		case 160: //increase armor value (impenetrability)
			{
			}
		case 161: //decrease armor value (brittlemail)
			{
			}
		case 162: //increase acid resistance
			{
			}
		case 163: //decrease acid resistance
			{
			}
		case 164: //increase bludgeon resistance
			{
			}
		case 165: //decrease bludgeon resistance
			{
			}
		case 166: //increase cold resistance
			{
			}
		case 167: //decrease cold resistance
			{
			}
		case 168: //increase lightning resistance
			{
			}
		case 169: //decrease lightning resistance
			{
			}
		case 170: //increase flame resistance
			{
			}
		case 171: //decrease flame resistance
			{
			}
		case 172: //increase piercing resistance
			{
			}
		case 173: //decrease piercing resistance
			{
			}
		case 174: //increase slashing resistance
			{
			}
		case 175: //decrease slashing resistance
			{
			}
		case 176: //lesser bludgeoning durance (increase bludgeon resistance)
			{
			}
		case 178: //lesser slashing durance (increase slashing resistance)
			{
			}
		case 180: //lesser piercing durance (increase piercing resistance)
			{
			}
		case 182: //greater stimulation durance (increase lightning resistance)
			{
			}
		case 184: //greater stasis durance (increase cold resistance)
			{
			}
		case 186: //greater consumption durance (increase fire resistance)
			{
			}
		case 188: //greater decay durance (increase acid resistance)
			{
			}
		case 190: //hieromancer's ward (increase armor value)
			{
			}
		case 192: //increase lock's pick resistance
			{
			}
		case 193: //decrease lock's pick resistance
			{
			}
		case 194: //increase item's appraisal resistance
			{
			}
		case 195: //decrease item's appraisal resistance
			{
			}
		case 200: //primary portal tie, secondary portal tie, lifestone tie
			{
			}
		case 201: //primary portal recall, secondary portal recall, lifestone recall, portal recall
			{
			}
		case 203: //portal summon spells
			{
			}
		case 214: //portal sending, specific portal recalls
			{
			}
		case 215: //lifestone sending
			{
			}
		case 250: //dispels
			{
			}
		case 323: //increase weapon's damage
			{
			}
		case 325: //increase bow's damage
			{
			}
		case 327: //increase bow's range
			{
			}
		case 329: //increase weapon's defense skill
			{
			}
		case 381: //major & minor acid bane
			{
			}
		case 383: //major & minor bludgeon bane
			{
			}
		case 385: //major & minor flame bane
			{
			}
		case 387: //major & minor cold bane
			{
			}
		case 389: //increase weapon's attack skill mod
			{
			}
		case 391: //increase item's armor (major & minor impen)
			{
			}
		case 393: //major & minor piercing bane
			{
			}
		case 395: //major & minor slashing bane
			{
			}
		case 397: //major & minor lightning bane
			{
			}
		case 399: //increase weapon's speed (major & minor)
			{
			}
		}
	}
}

/**
 *	Determines the heal amount of a Heal spell.
 *
 *	This function is called whenever a Heal spell is cast
 *	@param dwLevel - The level of the Heal Spell
 *	@return dwHeal - The amount of Health healed by the spell
 */
DWORD cAvatar::GetHealValue( DWORD dwLevel )
{
	int iHealMax, iHealMin;
	switch ( dwLevel )
	{
	case 1:
		{
			iHealMax = 15;
			iHealMin = 8;
			break;
		}
	case 2:
		{
			iHealMax = 20;
			iHealMin = 11;
			break;
		}
	case 3:
		{
			iHealMax = 30;
			iHealMin = 16;
			break;
		}
	case 4:
		{
			iHealMax = 50;
			iHealMin = 26;
			break;
		}
	case 5:
		{
			iHealMax = 75;
			iHealMin = 38;
			break;
		}
	case 6:
		{
			iHealMax = 100;
			iHealMin = 50;
			break;
		}
	case 7:
		{
			iHealMax = 125;
			iHealMin = 75;
			break;
		}
	}
	
	srand( timeGetTime( ) );
	DWORD dwHeal = rand() % (iHealMax - iHealMin) + iHealMin;
	
	return dwHeal;
}

/**
 *	Determines the infused amount of an Infuse Stamina spell.
 *
 *	This function is called whenever an Infuse Stamina spell is cast
 *	@param dwLevel - The level of the Infuse Stamina Spell
 *	@return dwStamina - The amount of Stamina infused by the spell
 */
DWORD cAvatar::GetStaminaValue( DWORD dwLevel )
{
	int iStaminaMax, iStaminaMin;
	switch ( dwLevel )
	{
	case 1:
		{
			iStaminaMax = 20;
			iStaminaMin = 11;
			break;
		}
	case 2:
		{
			iStaminaMax = 30;
			iStaminaMin = 16;
			break;
		}
	case 3:
		{
			iStaminaMax = 50;
			iStaminaMin = 26;
			break;
		}
	case 4:
		{
			iStaminaMax = 75;
			iStaminaMin = 38;
			break;
		}
	case 5:
		{
			iStaminaMax = 100;
			iStaminaMin = 51;
			break;
		}
	case 6:
		{
			iStaminaMax = 150;
			iStaminaMin = 76;
			break;
		}
	case 7:
		{
			iStaminaMax = 175;
			iStaminaMin = 100;
			break;
		}
	}
	
	srand( timeGetTime( ) );
	DWORD dwStamina = rand() % (iStaminaMax - iStaminaMin) + iStaminaMin;
	
	return dwStamina;
}

/**
 *	Determines the rate of vital replenishment after a Regeneration spell.
 *
 *	This function is called whenever a Regeneration spell is cast
 *	@param dwLevel - The level of the Regeneration Spell
 *	@return flValue - The new rate of regeneration for the vital with the spell
 */
float cAvatar::GetRegenIncValue( DWORD dwLevel )
{
	float flValue;
	switch( dwLevel )
	{
	case 1:
		{
			flValue = 1.10f;
			break;
		}
	case 2:
		{
			flValue = 1.25f;
			break;
		}
	case 3:
		{
			flValue = 1.40f;
			break;
		}
	case 4:
		{
			flValue = 1.55f;
			break;
		}
	case 5:
		{
			flValue = 1.70f;
			break;
		}
	case 6:
		{
			flValue = 1.85f;
			break;
		}
	case 7:
		{
			flValue = 2.15f;
			break;
		}
	}

	return flValue;		
}

/**
 *	Determines the rate of vital replenishment after a Fester spell.
 *
 *	This function is called whenever a Fester spell is cast
 *	@param dwLevel - The level of the Fester Spell
 *	@return flValue - The new rate of regeneration for the vital with the spell
 */
float cAvatar::GetRegenDecValue( DWORD dwLevel )
{
    float flValue;
	switch( dwLevel )
	{
	case 1:
		{
			flValue = 0.91f;
			break;
		}
	case 2:
		{
			flValue = 0.80f;
			break;
		}
	case 3:
		{
			flValue = 0.71f;
			break;
		}
	case 4:
		{
			flValue = 0.65f;
			break;
		}
	case 5:
		{
			flValue = 0.59f;
			break;
		}
	case 6:
		{
			flValue = 0.54f;
			break;
		}
	case 7:
		{
			flValue = 0.40f;
			break;
		}
	}

	return flValue;
}

/**
 *	Determines the damage percentage after a Protection spell
 *
 *	This function is called whenever a Protection spell is cast
 *	@param dwLevel - The level of the Protection Spell
 *	@return flValue - The new damage percentage for the damage type with the spell
 */
float cAvatar::GetProtValue( DWORD dwLevel )
{
    float flValue;
	switch( dwLevel )
	{
	case 1:
		{
			flValue = 0.91f;
			break;
		}
	case 2:
		{
			flValue = 0.80f;
			break;
		}
	case 3:
		{
			flValue = 0.67f;
			break;
		}
	case 4:
		{
			flValue = 0.57f;
			break;
		}
	case 5:
		{
			flValue = 0.50f;
			break;
		}
	case 6:
		{
			flValue = 0.40f;
			break;
		}
	case 7:
		{
			flValue = 0.35f;
			break;
		}
	}

	return flValue;
}

/**
 *	Determines the damage percentage after a Vulnerability spell
 *
 *	This function is called whenever a Vulnerability spell is cast
 *	@param dwLevel - The level of the Vulnerability Spell
 *	@return flValue - The new damage percentage for the damage type with the spell
 */
float cAvatar::GetVulnValue( DWORD dwLevel )
{
    float flValue;
	switch( dwLevel )
	{
	case 1:
		{
			flValue = 1.10f;
			break;
		}
	case 2:
		{
			flValue = 1.25f;
			break;
		}
	case 3:
		{
			flValue = 1.50f;
			break;
		}
	case 4:
		{
			flValue = 1.75f;
			break;
		}
	case 5:
		{
			flValue = 2.00f;
			break;
		}
	case 6:
		{
			flValue = 2.50f;
			break;
		}
	case 7:
		{
			flValue = 2.85f;
			break;
		}
	}

	return flValue;
}

/**
 *	Determines the increase in Armor Level after an Invulnerability spell
 *
 *	This function is called whenever an Invulnerability spell is cast
 *	@param dwLevel - The level of the Invulnerability Spell
 *	@return flValue - The increase in Armor Level by the spell
 */
float cAvatar::GetArmorValue( DWORD dwLevel )
{
    float flValue;
	switch( dwLevel )
	{
	case 1:
		{
			flValue = 20.0f;
			break;
		}
	case 2:
		{
			flValue = 50.0f;
			break;
		}
	case 3:
		{
			flValue = 75.0f;
			break;
		}
	case 4:
		{
			flValue = 100.0f;
			break;
		}
	case 5:
		{
			flValue = 150.0f;
			break;
		}
	case 6:
		{
			flValue = 200.0f;
			break;
		}
	case 7:
		{
			flValue = 225.0f;
			break;
		}
	}
	
	return flValue;
}

/**
 *	Determines the amount of vital transferred by a Vital to Vital spell
 *
 *	This function is called whenever a Vital to Vital spell is cast
 *	@param dwLevel - The level of the Vital to Vital Spell
 *	@return flValue - The percentage of vital transferred by the spell
 */
float cAvatar::GetTransferValue( DWORD dwLevel )
{
	float flValue;
	switch( dwLevel )
	{
	case 1:
		{
			flValue = 0.75f;
			break;
		}
	case 2:
		{
			flValue = 0.90f;
			break;
		}
	case 3:
		{
			flValue = 1.05f;
			break;
		}
	case 4:
		{
			flValue = 1.20f;
			break;
		}
	case 5:
		{
			flValue = 1.35f;
			break;
		}
	case 6:
		{
			flValue = 1.50f;
			break;
		}
	case 7:
		{
			flValue = 1.75f;
			break;
		}
	}

	return flValue;
}

/**
 *	Handles the message sent for the updating of an avatar's location.
 *
 *	@return cMessage - Returns an 0x0000F748 server message.
 */
cMessage cAvatar::SetPosition()
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

/**
 *	Handles the message sent for an avatar's recall to a lifestone.
 *
 *	@return cMessage - Returns a Toggle Object Visibility (0x0000F74B) server message.
 */
cMessage cAvatar::LifestoneRecall( )
{
	cMessage cmLifestone;
	cmLifestone << 0xF74B 
				<< m_dwGUID 
				<< WORD(0x0408)
				<< WORD(0x40)
				<< WORD(m_wNumLogins)
				<< WORD(m_wPortalCount++);
	return cmLifestone;
}
/**
 *	Handles the message sent for the avatar's animation when recalling to a lifestone.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::LSAnimate( )
{
	cMessage cmLifestone;

	cmLifestone		<< 0xF74C
					<< m_dwGUID
					<< m_wNumLogins
					<< ++m_wCurAnim
					<< ++m_wMeleeSequence
					<< WORD(0x0000)			//activity (idle)
					<< BYTE(0x00)			//animation type (general animation)
					<< BYTE(0x00)			//type flags (no target)
					<< WORD(0x003D)			//standing
					<< DWORD(0x02B4F080)	//flags
					<< WORD(0x0137)
					<< m_wMeleeSequence
					<< float(1)
					<< DWORD(0x023F4B41);

	return cmLifestone;
}
/**
 *	Handles the message sent for the server text when recalling to a lifestone.
 *
 *	@return cMessage - Returns a Server Text (0x0000F62C) server message.
 */
cMessage cAvatar::LSMessage( )
{
	cMessage cmLifestone;
	char	szTextBuffer[1024];
	sprintf(&szTextBuffer[0],"%s is recalling to the lifestone.", Name( ));

	DWORD dwColor = 0x17L;
	cmLifestone << 0xF62CL << szTextBuffer << dwColor;

	return cmLifestone;
}

/**
 *	Handles the message sent for an avatar's recall to the Marketplace.
 *
 *	@return cMessage - Returns a Toggle Object Visibility (0x0000F74B) server message.
 */
cMessage cAvatar::MarketplaceRecall( )
{
	cMessage cmMarketplace;
	cmMarketplace	<< 0xF74B 
					<< m_dwGUID 
					<< WORD(0x0408)
					<< WORD(0x40)
					<< WORD(m_wNumLogins)
					<< WORD(m_wPortalCount++);
	return cmMarketplace;
}
/**
 *	Handles the message sent for the avatar's animation when recalling to the Marketplace.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::MPAnimate( )
{
	cMessage cmMarketplace;

	cmMarketplace	<< 0xF74C
					<< m_dwGUID
					<< m_wNumLogins
					<< ++m_wCurAnim
					<< ++m_wMeleeSequence
					<< WORD(0x0000)			//activity (idle)
					<< BYTE(0x00)			//animation type (general animation)
					<< BYTE(0x00)			//type flags (no target)
					<< WORD(0x003D)			//standing
					<< DWORD(0x02B5F080)	//flags
					<< WORD(0x0137)
					<< m_wMeleeSequence
					<< float(1)
					<< DWORD(0x0BA9C719);

	return cmMarketplace;
}
/**
 *	Handles the message sent for the server text when recalling to the Marketplace.
 *
 *	@return cMessage - Returns a Server Text (0x0000F62C) server message.
 */
cMessage cAvatar::MPMessage( )
{
	cMessage cmMarketplace;
	char	szTextBuffer[1024];
	sprintf(&szTextBuffer[0],"%s is going to the Marketplace.", Name( ));

	DWORD dwColor = 0x17L;
	cmMarketplace << 0xF62CL << szTextBuffer << dwColor;

	return cmMarketplace;
}

/**
 *	Handles the message sent for an avatar's recall to a house.
 *
 *	@return cMessage - Returns a Toggle Object Visibility (0x0000F74B) server message.
 */
cMessage cAvatar::HouseRecall( )
{
	cMessage cmHouseRecall;
	cmHouseRecall	<< 0xF74B 
					<< m_dwGUID 
					<< WORD(0x0408)
					<< WORD(0x40)
					<< WORD(m_wNumLogins)
					<< WORD(m_wPortalCount++);
	return cmHouseRecall;
}
/**
 *	Handles the message sent for the avatar's animation when recalling to a house.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::HRAnimate( )
{
	cMessage cmHouseRecall;

	cmHouseRecall	<< 0xF74C
					<< m_dwGUID
					<< m_wNumLogins
					<< ++m_wCurAnim
					<< ++m_wMeleeSequence
					<< WORD(0x0000)			//activity (idle)
					<< BYTE(0x00)			//animation type (general animation)
					<< BYTE(0x00)			//type flags (no target)
					<< WORD(0x003D)			//standing
					<< DWORD(0x00000080)	//flags
					<< WORD(0x0137)			//animation
					<< m_wMeleeSequence
					<< float(1);

	return cmHouseRecall;

/* animations

0x0137 - recall
0x0085 - shake head
0x008B - scratch head
*/
}
/**
 *	Handles the message sent for the server text when recalling to a house.
 *
 *	@return cMessage - Returns a Server Text (0x0000F62C) server message.
 */
cMessage cAvatar::HRMessage( )
{
	cMessage cmHouseRecall;
	char	szTextBuffer[1024];
	sprintf(&szTextBuffer[0],"%s is recalling home.", Name( ));

	DWORD dwColor = 0x17L;
	cmHouseRecall << 0xF62CL << szTextBuffer << dwColor;

	return cmHouseRecall;
}

cMessage cAvatar::ConfirmPanelRequest(DWORD F7B0seq, DWORD type, DWORD ConfirmSeq, std::string targetName)
{
	cMessage cmConfirm;

	cmConfirm	<< 0xF7B0L << m_dwGUID << F7B0seq << 0x274L
				<< type
				<< ConfirmSeq
				<< targetName.c_str();

	return cmConfirm;
}

void cAvatar::SwearAllegiance(std::string szTargetName, DWORD dwReply, DWORD dwSenderGUID)
{
	cClient* pcClient = cClient::FindClient(m_dwGUID);
	cClient* pcSendClient = cClient::FindClient(dwSenderGUID);

	if (dwReply == 1)
	{
		if (pcClient)
		{
			cMessage	cmSwornMessage;
			char		szTextBuffer[255];
			sprintf(&szTextBuffer[0],"%s has sworn allegiance to you.", szTargetName.c_str());

			DWORD dwColor = 0x17L;
			cmSwornMessage << 0xF62CL << szTextBuffer << dwColor;

			pcClient->AddPacket( WORLD_SERVER, cmSwornMessage, 4 );
		}

		if (pcSendClient)
		{
			if (pcSendClient->m_pcAvatar)
			{
				// add the avatar to the allegiance
				cAvatar* pcSwornAvatar = pcSendClient->m_pcAvatar;
				cAllegiance::AddNewMember(pcSwornAvatar->GetGUID(), this->m_dwGUID, pcSwornAvatar->GetAllegianceID(), this->m_dwAllegianceID);
			}
		}
	} else
	{
	}

	// send the reply to the originating client
	if (pcSendClient)
		if (pcSendClient->m_pcAvatar)
			pcSendClient->m_pcAvatar->SwearAllegianceReply(m_strCharacterName, dwReply);
}

void cAvatar::SwearAllegianceReply(std::string szTargetName, DWORD dwReply)
{
	cClient* pcClient = cClient::FindClient(m_dwGUID);

	if (pcClient)
	{
		if (dwReply == 1)
		{
			cMessage	cmSwornMessage;
			char		szTextBuffer[255];
			sprintf(&szTextBuffer[0],"%s has accepted your oath of allegiance.", szTargetName.c_str());

			DWORD dwColor = 0x17L;
			cmSwornMessage << 0xF62CL << szTextBuffer << dwColor;

			pcClient->AddPacket( WORLD_SERVER, cmSwornMessage, 4 );

			// kneeling animation
			cMessage cmSAAnimation;
			cmSAAnimation		<< 0xF74C
								<< m_dwGUID
								<< m_wNumLogins
								<< ++m_wCurAnim
								<< ++m_wMeleeSequence
								<< WORD(0x0000)			//activity (idle)
								<< BYTE(0x00)			//animation type (general animation)
								<< BYTE(0x00)			//type flags (no target)
								<< WORD(0x003D)			//standing
								<< DWORD(0x000000C0)	//flags
								<< float(1.5)
								
								<< WORD(0x0092)
								//<< WORD(0x0009)			//changes	//0x0009	0x0053
								<< m_wMeleeSequence
								<< float(1);
/*						
								<< DWORD(0x01L)			//changes	//0x01L		0x03L
								<< WORD(0x99D6)			//changes	//0x74E8	//0x99D6
								<< WORD(0x0441)			//changes	//0x0444	//0x0441
								<< WORD(0x0001)
								<< DWORD(0x00000021)
								<< WORD(0x0004)
								<< WORD(0x02DA)
								<< WORD(0x0000)
								<< BYTE(0x02)			//changes	//0x00L		//0x02L
								<< dwTarget				//target GUID
								<< DWORD(0x0000001A)
								<< dwTarget;			//target GUID
*/
			cWorldManager::SendToAllInFocus( m_Location, cmSAAnimation, 3 );
		} else
		{
			cMessage	cmSwornMessage;
			char		szTextBuffer[255];
			sprintf(&szTextBuffer[0],"%s has declined your offer of allegiance.", szTargetName.c_str());

			DWORD dwColor = 0x17L;
			cmSwornMessage << 0xF62CL << szTextBuffer << dwColor;

			pcClient->AddPacket( WORLD_SERVER, cmSwornMessage, 4 );
		}
	}
}

void cAvatar::BreakAllegiance(DWORD dwBrokenMemberGUID)
{	
	cClient* pcClient = cClient::FindClient(m_dwGUID);
	cClient* pcReplyClient;

	cAllegiance* aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID);

	pcReplyClient = cClient::FindClient(dwBrokenMemberGUID);
	
	if (aAllegiance)
	{
		if (pcClient)
		{
			std::string szMemberName = aAllegiance->GetMemberName(dwBrokenMemberGUID);

			cMessage	cmBreakMessage;
			char		szTextBuffer[255];
			sprintf(&szTextBuffer[0],"You have broken your Allegiance to %s!", szMemberName.c_str());

			DWORD dwColor = 0x17L;
			cmBreakMessage << 0xF62CL << szTextBuffer << dwColor;

			pcClient->AddPacket( WORLD_SERVER, cmBreakMessage, 4 );
		}

		Member* thisMember = aAllegiance->FindMember(m_dwGUID);
		if (thisMember)
		{
			if (dwBrokenMemberGUID == thisMember->m_dwPatron)
			{	// broke with patron; remove oneself
				cAllegiance::RemMember(m_dwGUID, this->m_dwAllegianceID);
			}
			else
			{	// removed vassal/follower; remove that member
				cAllegiance::RemMember(dwBrokenMemberGUID, this->m_dwAllegianceID);
			}

			// send the reply to the player with whom the break occurred, if online
			if (pcReplyClient)
				pcReplyClient->m_pcAvatar->BreakAllegianceReply(this->Name());
		}
	}
}

void cAvatar::BreakAllegianceReply(std::string szMemberName)
{
	cClient* pcClient = cClient::FindClient(m_dwGUID);

	if (pcClient)
	{
		cMessage	cmBreakMessage;
		char		szTextBuffer[255];
		sprintf(&szTextBuffer[0],"%s has broken their Allegiance to you!", szMemberName.c_str());

		DWORD dwColor = 0x17L;
		cmBreakMessage << 0xF62CL << szTextBuffer << dwColor;

		pcClient->AddPacket( WORLD_SERVER, cmBreakMessage, 4 );
	}
}

cMessage cAvatar::Animation( WORD wAnim, float flPlaySpeed )
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
cMessage cAvatar::WarAnim( WORD wAnim, float flPlaySpeed )
{
	cMessage cWarAnim;
	WORD wAnimPart2;

	static BYTE CWarAnimation[] = { 
		0x4C, 0xF7, 0x00, 0x00, 
		0x47, 0x69, 0x02, 0x50, //character
		0x6B, 0x0A, //numLogins
		0x3B, 0x00, //Sequence
		0x0C, 0x00, //numanimations
		0x00, 0x00, //activity 0x0 idle , 0x1 active
		0x00, 0x00, //0x00, 0x00, //0x00, 0x00, //animation type 0 - general 6 - run to object 7 - run to position 8- face object 9-face position
		0x49, // Stance Mode 3C melee UA, 3D standing, 3E melee weapon, 3F bow, 40 melee weapon shield, 49 Spellcasting
		0x00,
		0x41, 0xF0, 0x9A, 0x02, //0x47, 0xF0, 0x9A, 0x02, //0x6D, 0x02, //Flags
		0x49, 0x00, //0x49, 0x00, // Stance 2
		0x00, 0x00, //0x33, 0x00, //0x33, 0x00, // animation 1
		0xC0, 0xBF, //0x00, 0x00,
		0x00, 0x00, //0x00, 0x40,
		//0x04, 0x00, //0x00, 0x00,
		//0x00, 0x00, //0xC0, 0xBF, //0x3F, // Speed Animation 1
		//0x00, 0x40, 0x00, 0x00,
	};

	++m_wCurAnim;

	CopyMemory( &CWarAnimation[04], &m_dwGUID, 4 );
	CopyMemory( &CWarAnimation[8], &m_wNumLogins, 2 );
	CopyMemory( &CWarAnimation[10], &m_wCurAnim, 2 );
	CopyMemory( &CWarAnimation[12], &m_wMeleeSequence, 2 );
	//CopyMemory( &CWarAnimation[24], &wAnim, 2 );
	//CopyMemory( &CWarAnimation[28], &flPlaySpeed, 4 );

	wAnimPart2 = m_wCurAnim - 6;
	//CopyMemory( &CWarAnimation[26], &wAnimPart2, 2 );
	
	cWarAnim.CannedData( CWarAnimation, sizeof( CWarAnimation ) );
	cWarAnim << WORD(0x0000);
	return cWarAnim;
}
//Spell Windup Animation
cMessage cAvatar::WindupAnimation( WORD wAnim1, float flPlaySpeed )
{
	cMessage cWindupAnim;
	WORD wAnimPart2;

	static BYTE CWindupAnimation[] = { 
		0x4C, 0xF7, 0x00, 0x00, 
		0x47, 0x69, 0x02, 0x50, //character
		0x6B, 0x0A, //numLogins
		0x3B, 0x00, //Sequence
		0x0C, 0x00, //numanimations
		0x00, 0x00, //activity 0x0 idle , 0x1 active
		0x00, 0x00, //animation type 0 - general 6 - run to object 7 - run to position 8- face object 9-face position
		0x49, // Stance Mode 3C melee UA, 3D standing, 3E melee weapon, 3F bow, 40 melee weapon shield, 49 Spellcasting
		0x00,
		0xC1, 0xF0, 0x9A, 0x02, //0x47, 0xF0, 0x6D, 0x02, //Flags
		0x49, 0x00, 
		0x00, 0x00, 
		0xC0, 0x3F,
		0x00, 0x00,
		0x04, 0x00, 
		0x00, 0x00,
		0x00, 0x40,
		0x00, 0x00,
	};

	++m_wCurAnim;

	CopyMemory( &CWindupAnimation[04], &m_dwGUID, 4 );
	CopyMemory( &CWindupAnimation[8], &m_wNumLogins, 2 );
	CopyMemory( &CWindupAnimation[10], &m_wCurAnim, 2 );
	CopyMemory( &CWindupAnimation[12], &m_wMeleeSequence, 2 );
	CopyMemory( &CWindupAnimation[24], &wAnim1, 2 );
	//CopyMemory( &CWindupAnimation[28], &flPlaySpeed, 4 );

	wAnimPart2 = m_wCurAnim - 6;
	//CopyMemory( &CWindupAnimation[26], &wAnimPart2, 2 );
	
	cWindupAnim.CannedData( CWindupAnimation, sizeof( CWindupAnimation ) );
	return cWindupAnim;
}

/**
 *	Handles the message sent for the avatar's animation when casting a war spell.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::WarAnimation( WORD wAnim1, float flPlaySpeed )
{
	cMessage cWarAnim;
	WORD wAnimPart2;

	static BYTE CWarAnimation[] = { 
		0x4C, 0xF7, 0x00, 0x00, 
		0x47, 0x69, 0x02, 0x50, //character
		0x6B, 0x0A, //numLogins
		0x3B, 0x00, //Sequence
		0x0C, 0x00, //numanimations
		0x00, 0x00, //activity 0x0 idle , 0x1 active
		0x00, 0x00, //animation type 0 - general 6 - run to object 7 - run to position 8- face object 9-face position
		0x49, // Stance Mode 3C melee UA, 3D standing, 3E melee weapon, 3F bow, 40 melee weapon shield, 49 Spellcasting
		0x00,
		0x47, 0xF0, 0x9A, 0x02, //0x47, 0xF0, 0x6D, 0x02, //Flags
		0x49, 0x00, 
		0x33, 0x00, // cast animation, wAnim1
		0x00, 0x00,
		0x00, 0x40,
		0x00, 0x00, 
		0xC0, 0xBF, //0xC0, 0x3F, // Speed Animation 1
	};

	++m_wCurAnim;

	CopyMemory( &CWarAnimation[04], &m_dwGUID, 4 );
	CopyMemory( &CWarAnimation[8], &m_wNumLogins, 2 );
	CopyMemory( &CWarAnimation[10], &m_wCurAnim, 2 );
	CopyMemory( &CWarAnimation[12], &m_wMeleeSequence, 2 );
	CopyMemory( &CWarAnimation[26], &wAnim1, 2 );
	//CopyMemory( &CWarAnimation[28], &flPlaySpeed, 4 );

	wAnimPart2 = m_wCurAnim - 6;
	//CopyMemory( &CWarAnimation[26], &wAnimPart2, 2 );
	
	cWarAnim.CannedData( CWarAnimation, sizeof( CWarAnimation ) );
	//cWarAnim << WORD(0x0000);
	return cWarAnim;
}
/**
 *	Handles the message sent for the avatar's animation when casting a war spell.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::WarAnimation2( float flPlaySpeed )
{
	cMessage cWarAnim;
	WORD wAnimPart2;

	static BYTE CWarAnimation[] = { 
		0x4C, 0xF7, 0x00, 0x00, 
		0x47, 0x69, 0x02, 0x50, //character
		0x6B, 0x0A, //numLogins
		0x3B, 0x00, //Sequence
		0x0C, 0x00, //numanimations
		0x00, 0x00, //activity 0x0 idle , 0x1 active
		0x00, 0x00, //animation type 0 - general 6 - run to object 7 - run to position 8- face object 9-face position
		0x49, 0x00, // Stance Mode 3C melee UA, 3D standing, 3E melee weapon, 3F bow, 40 melee weapon shield, 49 Spellcasting
		0x41, 0xF0, 0x9A, 0x02, //0x41, 0xF0, 0x6D, 0x02, //Flags
		0x49, 0x00, // Stance 2
		0x00, 0x00,
		0xC0, 0xBF, 0x00, 0x00, //0xC0, 0x3F, 0x00, 0x00,
	};

	++m_wCurAnim;

	CopyMemory( &CWarAnimation[04], &m_dwGUID, 4 );
	CopyMemory( &CWarAnimation[8], &m_wNumLogins, 2 );
	CopyMemory( &CWarAnimation[10], &m_wCurAnim, 2 );
	CopyMemory( &CWarAnimation[12], &m_wMeleeSequence, 2 );
//	CopyMemory( &CWarAnimation[28], &flPlaySpeed, 4 );

	wAnimPart2 = m_wCurAnim - 6;
	//CopyMemory( &CWarAnimation[26], &wAnimPart2, 2 );
	
	cWarAnim.CannedData( CWarAnimation, sizeof( CWarAnimation ) );
	return cWarAnim;
}

cMessage cAvatar::AddEnchant(DWORD F7B0seq, DWORD dwSpellID, WORD wEnchantLayer, WORD wSpellFamily, DWORD dwDifficulty, double dDuration, DWORD dwCasterGUID, double dCastTime, DWORD dwFlags, DWORD dwKey, float flValue)
{
	cMessage cmEnchant;

	cmEnchant	<< 0xF7B0L 
				<< GetGUID() 
				<< F7B0seq 
				<< 0x004EL
				<< (WORD) dwSpellID
				<< wEnchantLayer
				<< wSpellFamily
				<< (WORD) 0x0001 //unknown0
				<< dwDifficulty
				<< (double) (timeGetTime() - dCastTime)
				<< dDuration
				<< dwCasterGUID
				<< 0x0L //unknown1
				<< 0xC4268000 //unknown2 (-666), for Vitae it's 3F800000 (1)
				<< dCastTime
				<< dwFlags //Vitae - 0x00A06012, Armor self - 0x0000A080, attributes (focus) - 0x00009001, skills - 0x00009010, all stats/attr/skills - 0x0000A001, all skills - 0x0000A010, prots/regen - 0x00005008
				<< dwKey //prots: blade 40, pierce 41, bludgeon 42, fire 43, cold 44, acid 45, lightning 46; regens: health 3, stamina 4, mana 5
				<< flValue 
				<< 0x0L; //unknown3

	return cmEnchant;
}
/**
 *	Handles the message sent to the avatar when a spell timer expires.
 *
 *	@return cMessage - Returns an 0x0000F7B0 server message.
 */
cMessage cAvatar::RemoveEnchant(DWORD F7B0seq, DWORD dwSpellID, WORD wEnchantLayer)
{
	cMessage cmEnchant;

	cmEnchant	<< 0xF7B0L
				<< GetGUID()
				<< F7B0seq
				<< 0x004FL
				<< dwSpellID
				<< wEnchantLayer;

	return cmEnchant;
}

// Start Combat Routines 

cMessage cAvatar::ChangeCombatMode(BOOL fMode, BYTE WieldType)
{
	
	cMessage cmRet;

	if(fMode==TRUE)
	{
		switch(WieldType)
		{
			//Unarmed
		case 0x00:
			{
				unsigned char Canim4 [] = {
					0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3D, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x3C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 
				};
				CopyMemory(&Canim4[4],&m_dwGUID,4);
				CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
				m_wCurAnim++;
				CopyMemory(&Canim4[10],&m_wCurAnim,2);
				m_wMeleeSequence++;
				CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
				cmRet.pasteData(Canim4,sizeof(Canim4));
				break;
			}
			//Melee Weapon
		case 0x01:
			{
					unsigned char Canim4 [] = 
					{
						0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3D, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 
						0x00, 0x00, 0x3E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 
					};
					CopyMemory(&Canim4[4],&m_dwGUID,4);
					CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
					m_wCurAnim++;
					CopyMemory(&Canim4[10],&m_wCurAnim,2);
					m_wMeleeSequence++;
					CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
					cmRet.pasteData(Canim4,sizeof(Canim4));
					break;
			}

		}
	}
	else
	{
		unsigned char Canim4 [] = {
			0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x49, 0x00, 0x08, 0x00, 0x02, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 
		};
		CopyMemory(&Canim4[4],&m_dwGUID,4);
		CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
		m_wCurAnim++;
		CopyMemory(&Canim4[10],&m_wCurAnim,2);
		m_wMeleeSequence++;
		CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
		cmRet.pasteData(Canim4,sizeof(Canim4));
	}
	
	return cmRet;
}

cMessage cAvatar::ChangeSpellMode(BOOL fMode)
{
	cMessage cmRet;
	if(fMode==TRUE)
	{
		unsigned char Canim4 [] = {
			0x4C, 0xF7, 0x00, 0x00, 0x47, 0x69, 0x02, 0x50, 0x6C, 0x0A, 0x14, 0x00, 0x04, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x49, 0x00, 0x01, 0xF0, 0x99, 0x02, 0x49, 0x00, 0x00, 0x00,
		};
		CopyMemory(&Canim4[4],&m_dwGUID,4);
		CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
		m_wCurAnim++;
		CopyMemory(&Canim4[10],&m_wCurAnim,2);
		m_wMeleeSequence++;
		CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
		cmRet.pasteData(Canim4,sizeof(Canim4));
	}
	else
	{
		unsigned char Canim4 [] = {
			0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x49, 0x00, 0x08, 0x00, 0x02, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 
		};
		CopyMemory(&Canim4[4],&m_dwGUID,4);
		CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
		m_wCurAnim++;
		CopyMemory(&Canim4[10],&m_wCurAnim,2);
		m_wMeleeSequence++;
		CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
		cmRet.pasteData(Canim4,sizeof(Canim4));
	}
	return cmRet;
}

cMessage  cAvatar::ChangeMissileMode(BOOL fMode, WORD AmmoType)
{
	cMessage cmRet;
	if(fMode == TRUE)
	{
		switch(AmmoType)
		{
		case 0x0001:
			{
				//Equip animation
				unsigned char Canim4 [] = 
				{
					0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3D, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x3F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 
				};
				CopyMemory(&Canim4[4],&m_dwGUID,4);
				CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
				m_wCurAnim++;
				CopyMemory(&Canim4[10],&m_wCurAnim,2);
				m_wMeleeSequence++;
				CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
				cmRet.pasteData(Canim4,sizeof(Canim4));

				//Grabbing arrow animation
				unsigned char Canim5 [] = 
				{
					0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3D, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 
				};
				CopyMemory(&Canim5[4],&m_dwGUID,4);
				CopyMemory( &Canim5[8], &m_wNumLogins, 2 );
				m_wCurAnim++;
				CopyMemory(&Canim5[10],&m_wCurAnim,2);
				m_wMeleeSequence++;
				CopyMemory(&Canim5[12],&m_wMeleeSequence,2);
				cmRet.pasteData(Canim5,sizeof(Canim5));
				break;
			}
		case 0x0002:
			{
				//Equip animation
				unsigned char Canim4 [] = 
				{
					0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3D, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x41, 0x00, 0x01, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 
				};
				CopyMemory(&Canim4[4],&m_dwGUID,4);
				CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
				m_wCurAnim++;
				CopyMemory(&Canim4[10],&m_wCurAnim,2);
				m_wMeleeSequence++;
				CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
				cmRet.pasteData(Canim4,sizeof(Canim4));

				//Grabbing arrow animation
				unsigned char Canim5 [] = 
				{
					0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3D, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 
				};
				CopyMemory(&Canim5[4],&m_dwGUID,4);
				CopyMemory( &Canim5[8], &m_wNumLogins, 2 );
				m_wCurAnim++;
				CopyMemory(&Canim5[10],&m_wCurAnim,2);
				m_wMeleeSequence++;
				CopyMemory(&Canim5[12],&m_wMeleeSequence,2);
				cmRet.pasteData(Canim5,sizeof(Canim5));
				break;
			}
		}
	}
	else
	{
		unsigned char Canim4 [] = {
			0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x49, 0x00, 0x08, 0x00, 0x02, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 
		};
		CopyMemory(&Canim4[4],&m_dwGUID,4);
		CopyMemory( &Canim4[8], &m_wNumLogins, 2 );
		m_wCurAnim++;
		CopyMemory(&Canim4[10],&m_wCurAnim,2);
		m_wMeleeSequence++;
		CopyMemory(&Canim4[12],&m_wMeleeSequence,2);
		cmRet.pasteData(Canim4,sizeof(Canim4));
	}
	return cmRet;
}
/**
 *	Handles the message sent for the avatar's animation when combating.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::CombatAnimation(DWORD dwTarget)
{
	cMessage cmRet;
	/*
	unsigned char Canimation [] = {
		0x4C, 0xF7, 0x00, 0x00, 0xE6, 0xD2, 0x09, 0x50, 0x3C, 0x00, 0x18, 0x00, 0x03, 0x00, 0x00, 0x00, 
		0x00, 0x01, 0x00, 0x00, 0xC1, 0x80, 0x99, 0xB1, 0x3E, 0x00, 0x00, 0x00, 0x10, 0xC0, 0x67, 0x00, 
		0x01, 0x00, 0xE4, 0x38, 0x8E, 0x3F, 0x00, 0x00, 0x20, 0x8B, 0x99, 0xB1, 
	};
	CopyMemory(&Canimation[4],&m_dwGUID,4);
	CopyMemory( &Canimation[8], &m_wNumLogins, 2 );
	m_wCurAnim++;
	CopyMemory(&Canimation[10],&m_wCurAnim,2);
	m_wMeleeSequence++;
	CopyMemory(&Canimation[12],&m_wMeleeSequence,2);
	CopyMemory(&Canimation[0x28],&dwTarget,4);
	CopyMemory(&Canimation[0x20],&m_wCurAnim,2);
	cmRet.pasteData(Canimation,sizeof(Canimation));
	*/
	cmRet << 0xF74CL << this->m_dwGUID << this->m_wNumLogins << ++this->m_wCurAnim
		<< ++this->m_wMeleeSequence 
		<< WORD(0x0000)				//Active
		<< BYTE(0x00)				//General animation
		<< BYTE(0x01)				//Has target
		<< WORD(0x003E)				//Stance
		<< DWORD(0x00C1)			//Flags
		<< WORD(0x003E)				//Stance
		<< WORD(0xC010)
		<< WORD(0x0060)
		<< this->m_wCurAnim
		<< float(1.0)
		<< WORD(0x0000)
		<< dwTarget;

	return cmRet;
}

cMessage cAvatar::DoDamageMessage(DWORD F7B0seq,std::string target,DWORD damagetype,double severity,DWORD amount)
{
	cMessage cmRet;
	cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x01B1L << target.c_str() << damagetype << severity << amount;
	return cmRet;
}

cMessage cAvatar::RecieveDamageMessage(DWORD F7B0seq,std::string giver,DWORD damagetype,double severity,DWORD amount,DWORD location)
{
	cMessage cmRet;
	cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x01B2L << giver.c_str() << damagetype << severity << amount << location;
	return cmRet;
}

cMessage cAvatar::AttackBeginMessage(DWORD F7B0seq, DWORD pcGUID)
{
	//cClient m_pcClient;
	cMessage cmRet;
	cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x01B8L;
	return cmRet;
}


cMessage cAvatar::AttackCompleteMessage(DWORD F7B0seq)
{
	cMessage cmRet;
	cmRet << 0xF7B0L << m_dwGUID << F7B0seq << 0x01A7L << 0x37L;
	return cmRet;
}

//////////////////////////////////////
//	Stats updates		
//////////////////////////////////////

/**
 *	Handles the message sent to the avatar when experience is awarded to the unassigned experience value.
 *
 *	@param amount - The amount of experience to award.
 */
void cAvatar::UpdateHuntingExp(DWORD amount)
{
	cFellowship* aFellowship;
	if (aFellowship = cFellowship::GetFellowshipByID(m_dwFellowshipID))
	{
		aFellowship->DistributeXP(this->GetGUID(), this->m_Location, amount);
	}
	else
	{
		cAllegiance* aAllegiance;
		if (aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID))
		{
			//Find the avatar's allegiance record
			Member* memAvatar = &aAllegiance->members.find(m_dwGUID)->second;
			aAllegiance->VassalPassupXP(memAvatar, amount, false);
		}

		cClient* pcClient = cClient::FindClient(m_dwGUID);
		pcClient->AddPacket( WORLD_SERVER, UpdateUnassignedExp(amount), 4 );
	}
}

/**
 *	Handles the message sent to the avatar when experience is awarded through a fellowship.
 *
 *	@param amount - The amount of experience to award.
 */
void cAvatar::UpdateFellowshipExp(DWORD amount)
{
	cAllegiance* aAllegiance;
	if (aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID))
	{
		//Find the avatar's allegiance record
		Member* memAvatar = &aAllegiance->members.find(m_dwGUID)->second;
		aAllegiance->VassalPassupXP(memAvatar, amount, false);
	}
		
	cClient* pcClient = cClient::FindClient(m_dwGUID);
	pcClient->AddPacket( WORLD_SERVER, UpdateUnassignedExp(amount), 4 );
}

/**
 *	Handles the message sent to the avatar when experience is awarded to the unassigned experience value.
 *
 *	@param amount - The amount of experience to award.
 *
 *	@return cMessage - Returns an Update Statistic (0x00000237) server message.
 */
cMessage cAvatar::UpdateUnassignedExp(DWORD amount)
{
	DWORD xpAmount = AwardUnassignedXP(amount);
	cMessage cmRet;
	cmRet << 0x237L << m_bStatSequence++ << 0x16L << xpAmount;
	return cmRet;
}

/**
 *	Handles the message sent to the avatar when experience is decremented from the unassigned experience value.
 *
 *	@param amount - The amount of experience to decrement.
 *
 *	@return cMessage - Returns an Update Statistic (0x00000237) server message.
 */
cMessage cAvatar::DecrementUnassignedExp(DWORD amount)
{
	DWORD xpAmount = SpendUnassignedXP(amount);
	cMessage cmRet;
	cmRet << 0x237L << m_bStatSequence++ << 0x16L << xpAmount;
	return cmRet;
}

/**
 *	Handles the message sent to the avatar when experience is awarded to the total experience value.
 *
 *	@param amount - The amount of experience to award.
 *
 *	@return cMessage - Returns an Update Statistic (0x00000237) server message.
 */
cMessage cAvatar::UpdateTotalExp(DWORD amount)
{
	DWORD xpAmount = AwardTotalExp(amount);
	cMessage cmRet;
	cmRet << 0x237L << m_bStatSequence++ << 0x15L << xpAmount;
	return cmRet;
}

/**
 *	Handles the message sent to the avatar when experience is awarded to skill experience value.
 *
 *	@param amount - The amount of experience to award.
 *	@param skill - The skill to which the experience is awarded.
 *
 *	@return cMessage - Returns an Update Statistic (0x00000237) server message.
 */
cMessage cAvatar::UpdateSkillExp(DWORD amount, BYTE skill)
{
	DWORD xpAmount = AwardUnassignedXP(amount);
	cMessage cmRet;
	cmRet << 0x237L << m_bStatSequence++ << 0x18L << skill << xpAmount;
	return cmRet;
}

/**
 *	Handles the message sent when an avatar's pyreal amount changes.
 *
 *	@param amount - The amount of pyreals to add or remove.
 *	@param positive - Whether the amount is positive or negative
 *
 *	@return cMessage - Returns an Update Statistic (0x00000237) server message.
 */
cMessage cAvatar::UpdatePyreals(DWORD amount, int positive)
{
	DWORD pyrealsAmount;
	if (positive == 1)
		pyrealsAmount = AddPyreals(amount);
	else
		pyrealsAmount = RemovePyreals(amount);
	cMessage cmRet;
	cmRet << 0x237L << m_bStatSequence++ << 0x14L << DWORD(pyrealsAmount);
	return cmRet;
}

/**
 *	Handles the message sent when an avatar's burden amount changes.
 *
 *	@param amount - The amount of burden to add or remove.
 *	@param positive - Whether the amount should add or remove burden.
 *
 *	@return cMessage - Returns an Update Statistic (0x00000237) server message.
 */
cMessage cAvatar::UpdateBurden(DWORD amount, int action)
{
	DWORD burdenAmount;
	if (action == 0)
	{
		//Add Burden
		burdenAmount = AddBurden(amount);
	}
	else
	{
		burdenAmount = RemoveBurden(amount);
	}

	cMessage cmRet;
	cmRet << 0x237L << m_bStatSequence++ << 0x05L << burdenAmount;
	return cmRet;
}

/**
 *	Calculates the total base value of a skill.
 *
 *	The base value will be equal to the creation value plus creation bonus (if trained or specialized) plus skill increments plus any attribute bonus to the skill.
 *	The resultant value is used for server-side calculations; the client maintains its own value (which should match the server's value).
 *
 *	@param skillID - The numeric representation of the skill.
 */
void cAvatar::CalcSkill(DWORD skillID)
{
	int skillArrayNum = skillID;
	
	int total_skill;
	int skillBonus = 0;

	if (m_cStats.m_lpcSkills[skillArrayNum].m_wStatus == 3)
		m_cStats.m_lpcSkills[skillArrayNum].m_dwBonus = 10;
	else if (m_cStats.m_lpcSkills[skillArrayNum].m_wStatus == 2)
		m_cStats.m_lpcSkills[skillArrayNum].m_dwBonus = 5;
	else
		m_cStats.m_lpcSkills[skillArrayNum].m_dwBonus = 0;

	total_skill = this->m_cStats.m_lpcSkills[skillArrayNum].m_dwIncreases + m_cStats.m_lpcSkills[skillArrayNum].m_dwBonus;

	//Add the attribute bonuses
	switch(skillID)
	{
		//The client rounds the result of attribute contributions to the skill level.
		//Given that high accuracy is not required, flooring the value plus one-half is precise enough.

		case 0x01:	//Axe
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x02:	//Bow
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 2 + .5);
			}
			break;
		case 0x03:	//Crossbow
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 2 + .5);
			}
			break;
		case 0x04:	//Dagger
			{
				total_skill += floor((double)(this->base_quickness + this->m_cStats.m_lpcAttributes[2].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x05:	//Mace
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x06:	//Melee Defense
			{
				total_skill += floor((double)(this->base_quickness + this->m_cStats.m_lpcAttributes[2].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x07:	//Missile Defense
			{
				total_skill += floor((double)(this->base_quickness + this->m_cStats.m_lpcAttributes[2].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 5 + .5);
			}
			break;
		case 0x08:
			{
			}
			break;
		case 0x09:	//Spear
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x0A:	//Staff
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x0B:	//Sword
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x0C:	//Thrown Weapons
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 2 + .5);
			}
			break;
		case 0x0D:	//Unarmed Combat
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x0E:	//Arcane Lore
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x0F:	//Magic Defense
			{
				total_skill += floor((double)(this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement + this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 7 + .5);
			}
			break;
		case 0x10:	//Mana Conversion
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 6 + .5);
			}
			break;
		case 0x11:
			{
			}
			break;
		case 0x12:	//Appraise Item
			{
				total_skill += 0;
			}
			break;
		case 0x13:	//Assess Person
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 2 + .5);
			}
			break;
		case 0x14:	//Deception
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 4 + .5);
			}
			break;
		case 0x15:	//Healing
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x16:	//Jump
			{
				total_skill += floor((double)(this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement + this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement) / 2 + .5);
			}
			break;
		case 0x17:	//Lockpick
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement + this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x18:	//Run
			{
				total_skill += this->base_quickness + this->m_cStats.m_lpcAttributes[2].m_dwIncrement;
			}
			break;
		case 0x19:
			{
			}
			break;
		case 0x1A:
			{
			}
			break;
		case 0x1B:	//Assess Creature
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 2 + .5);
			}
			break;
		case 0x1C:	//Appraise Weapon
			{
				total_skill += 0;
			}
			break;
		case 0x1D:	//Appraise Armor
			{
				total_skill += 0;
			}
			break;
		case 0x1E:	//Appraise Magic Item
			{
				total_skill += 0;
			}
			break;
		case 0x1F:	//Creature Enchantment
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 4 + .5);
			}
			break;
		case 0x20:	//Item Enchantment
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 4 + .5);
			}
			break;
		case 0x21:	//Life Magic
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 4 + .5);
			}
			break;
		case 0x22:	//War Magic
			{
				total_skill += floor((double)(this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement + this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement) / 4 + .5);
			}
			break;
		case 0x23:	//Leadership
			{
				total_skill += 0;
			}
			break;
		case 0x24:	//Loyalty
			{
				total_skill += 0;
			}
			break;
		case 0x25:	//Fletching
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement + this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x26:	//Alchemy
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement + this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 3 + .5);
			}
			break;
		case 0x27:	//Cooking
			{
				total_skill += floor((double)(this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement + this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement) / 3 + .5);
			}
			break;
	}
	m_cStats.m_lpcSkills[skillArrayNum].m_dwTotal = total_skill;
}

//////////////////////////////////////
//	Skill updates		
//////////////////////////////////////

//String arrays used for updating skills
std::string	skillSQL[0x29] = {"Unknown1Inc","AxeInc","BowInc","CrossbowInc","DaggerInc","MaceInc","MeleeDefenseInc",
		"MissileDefenseInc","Unknown2Inc","SpearInc","StaffInc","SwordInc","ThrownWeaponsInc","UnarmedCombatInc",
		"ArcaneLoreInc","MagicDefenseInc","ManaConversionInc","Unknown3Inc","AppraiseItemInc","AssessPersonInc",
		"DeceptionInc","HealingInc","JumpInc","LockpickInc","RunInc","Unknown4Inc","Unknown5Inc","AssessCreatureInc",
		"AppraiseWeaponInc","AppraiseArmorInc","AppraiseMagicItemInc","CreatureEnchantmentInc","ItemEnchantmentInc",
		"LifeMagicInc","WarMagicInc","LeadershipInc","LoyaltyInc","FletchingInc","AlchemyInc","CookingInc"};
std::string	skillName[0x29] = {"Unknown1","Axe","Bow","Crossbow","Dagger","Mace","Melee Defense","Missile Defense",
		"Unknown2","Spear","Staff","Sword","Thrown Weapons","Unarmed Combat","Arcane Lore","Magic Defense","Mana Conversion",
		"Unknown3","Appraise Item","Assess Person","Deception","Healing","Jump","Lockpick","Run","Unknown4","Unknown5",
		"Assess Creature","Appraise Weapon","Appraise Armor","Appraise Magic Item","Creature Enchantment",
		"Item Enchantment","Life Magic","War Magic","Leadership","Loyalty","Fletching","Alchemy","Cooking"};

/**
 *	Handles the message sent when an avatar increments a skill.
 *	Also updates the database value.
 *
 *	@param skillID - The numeric representation of the skill. The function generically switches depending upon the skillID.
 *	@param exp - The amount of experience spent incrementing the skill. 
 *
 *	@return cMessage - Returns a Skill Experience (0x0000023E) server message.
 */
cMessage cAvatar::UpdateSkill(DWORD skillID, DWORD exp)
{
	char	szCommand[512];
	RETCODE	retcode;

	this->m_cStats.m_lpcSkills[skillID].m_dwIncreases++;
	this->m_cStats.m_lpcSkills[skillID].m_dwTotal++;
	m_cStats.m_lpcSkills[skillID].m_dwXP += exp;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar_skills SET %s = %i WHERE AvatarGUID = %lu;",skillSQL[skillID].c_str(), this->m_cStats.m_lpcSkills[skillID].m_dwIncreases,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Skill message
	cMessage cmUpdateSkill;
	cmUpdateSkill << 0x23EL << WORD(skillID) << BYTE(0x00) << BYTE(0x00) << WORD(this->m_cStats.m_lpcSkills[skillID].m_dwIncreases) << WORD(0x0001) << DWORD(this->m_cStats.m_lpcSkills[skillID].m_wStatus) << m_cStats.m_lpcSkills[skillID].m_dwXP << m_cStats.m_lpcSkills[skillID].m_dwBonus << DWORD(0x0L) << DWORD(0x0L) << DWORD(0x0L);
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base %s skill is now %d!",skillName[skillID].c_str(),this->m_cStats.m_lpcSkills[skillID].m_dwTotal );
	return cmUpdateSkill;
}

//////////////////////////////////////
//	Attribute updates		
//////////////////////////////////////

/**
 *	Handles the message sent when an avatar's Strength attribute is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Strength. 
 *
 *	@return cMessage - Returns an Update Attribute (0x00000241) server message.
 */
cMessage cAvatar::UpdateStrength(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcAttributes[0].m_dwIncrement++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET Strength = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcAttributes[0].m_dwIncrement,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	strength_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Str_exp_spent = %i WHERE AvatarGUID = %lu;",strength_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Strength message
	cMessage cmUpdStr;
	cmUpdStr << 0x241L << 0x01L << this->m_cStats.m_lpcAttributes[0].m_dwIncrement << this->m_cStats.m_lpcAttributes[0].m_dwCurrent << strength_exp_cost;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	//TODO: It seems that base_strength cannot be accessed (is equal to 0) because it is private?  Why?  (m_lpcAttributes entry used instead.)
	//cMasterServer::ServerMessage(ColorBlue,NULL,"Your Base Strength is now %d!",this->base_strength + this->m_cStats.m_lpcAttributes[0].m_dwIncrement);
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Strength is now %d!",this->m_cStats.m_lpcAttributes[0].m_dwCurrent + this->m_cStats.m_lpcAttributes[0].m_dwIncrement);
	
	//Skills affected by Strength
	CalcSkill(0x0001);	//Axe
	CalcSkill(0x0005);	//Mace
	CalcSkill(0x0009);	//Spear
	CalcSkill(0x000A);	//Staff
	CalcSkill(0x000B);	//Sword
	CalcSkill(0x000C);	//Thrown Weapons
	CalcSkill(0x000D);	//Unarmed Combat
	CalcSkill(0x0016);	//Jump
	CalcSkill(0x001C);	//Appraise Weapon / Weapon Tinkering

	return cmUpdStr;
}
/**
 *	Handles the message sent when an avatar's Endurance attribute is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Endurance. 
 *
 *	@return cMessage - Returns an Update Attribute (0x00000241) server message.
 */
cMessage cAvatar::UpdateEndurance(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcAttributes[1].m_dwIncrement++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET Endurance = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcAttributes[1].m_dwIncrement,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	endurance_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET End_exp_spent = %i WHERE AvatarGUID = %lu;",endurance_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Endurance message
	cMessage cmUpdEnd;
	cmUpdEnd << 0x241L << 0x02L << this->m_cStats.m_lpcAttributes[1].m_dwIncrement << this->m_cStats.m_lpcAttributes[1].m_dwCurrent << endurance_exp_cost;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Endurance is now %d!",this->base_endurance + this->m_cStats.m_lpcAttributes[1].m_dwIncrement);
	
	//Skills affected by Endurance
	CalcSkill(0x001D);	//Appraise Armor / Armor Tinkering

	//Vitals affected by Endurance
	CalcVital(0x0000);	//Health
	CalcVital(0x0001);	//Stamina
	
	return cmUpdEnd;
}
/**
 *	Handles the message sent when an avatar's Quickness attribute is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Quickness. 
 *
 *	@return cMessage - Returns an Update Attribute (0x00000241) server message.
 */
cMessage cAvatar::UpdateQuickness(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcAttributes[2].m_dwIncrement++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET Quickness = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcAttributes[2].m_dwIncrement,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	quickness_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Qui_exp_spent = %i WHERE AvatarGUID = %lu;",quickness_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Quickness message
	cMessage cmUpdQuick;
	cmUpdQuick << 0x241L << 0x03L << this->m_cStats.m_lpcAttributes[2].m_dwIncrement << this->m_cStats.m_lpcAttributes[2].m_dwCurrent << quickness_exp_cost;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Quickness is now %d!",this->base_quickness + this->m_cStats.m_lpcAttributes[2].m_dwIncrement);
	
	//Skills affected by Quickness
	CalcSkill(0x0004);	//Dagger
	CalcSkill(0x0006);	//Melee Defense
	CalcSkill(0x0007);	//Missile Defense
	CalcSkill(0x0018);	//Run

	return cmUpdQuick;
}
/**
 *	Handles the message sent when an avatar's Coordination attribute is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Coordination. 
 *
 *	@return cMessage - Returns an Update Attribute (0x00000241) server message.
 */
cMessage cAvatar::UpdateCoordination(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcAttributes[3].m_dwIncrement++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET Coordination = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcAttributes[3].m_dwIncrement,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	coordination_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Cor_exp_spent = %i WHERE AvatarGUID = %lu;",coordination_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Coordination message
	cMessage cmUpdCoord;
	cmUpdCoord << 0x241L << 0x04L << this->m_cStats.m_lpcAttributes[3].m_dwIncrement << this->m_cStats.m_lpcAttributes[3].m_dwCurrent << coordination_exp_cost;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Coordination is now %d!",this->base_coordination + this->m_cStats.m_lpcAttributes[3].m_dwIncrement);
	
	//Skills affected by Coordination
	CalcSkill(0x0001);	//Axe
	CalcSkill(0x0002);	//Bow
	CalcSkill(0x0003);	//Crossbow
	CalcSkill(0x0004);	//Dagger
	CalcSkill(0x0005);	//Mace
	CalcSkill(0x0006);	//Melee Defense
	CalcSkill(0x0007);	//Missile Defense
	CalcSkill(0x0009);	//Spear
	CalcSkill(0x000A);	//Staff
	CalcSkill(0x000B);	//Sword
	CalcSkill(0x000C);	//Thrown Weapons
	CalcSkill(0x000D);	//Unarmed Combat
	CalcSkill(0x0012);	//Appraise Item / Item Tinkering
	CalcSkill(0x0015);	//Healing
	CalcSkill(0x0016);	//Jump
	CalcSkill(0x0017);	//Lockpick
	CalcSkill(0x0026);	//Alchemy
	CalcSkill(0x0027);	//Cooking
	CalcSkill(0x0025);	//Fletching

	return cmUpdCoord;
}
/**
 *	Handles the message sent when an avatar's Focus attribute is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Focus. 
 *
 *	@return cMessage - Returns an Update Attribute (0x00000241) server message.
 */
cMessage cAvatar::UpdateFocus(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcAttributes[4].m_dwIncrement++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET Focus = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcAttributes[4].m_dwIncrement,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	focus_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Foc_exp_spent = %i WHERE AvatarGUID = %lu;",focus_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Focus message
	cMessage cmUpdFocus;
	cmUpdFocus << 0x241L << 0x05L << this->m_cStats.m_lpcAttributes[4].m_dwIncrement << this->m_cStats.m_lpcAttributes[4].m_dwCurrent << focus_exp_cost;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Focus is now %d!",this->base_focus + this->m_cStats.m_lpcAttributes[4].m_dwIncrement);

	//Skills affected by Focus
	CalcSkill(0x000E);	//Arcane Lore
	CalcSkill(0x000F);	//Magic Defense
	CalcSkill(0x0010);	//Mana Conversion
	CalcSkill(0x0012);	//Appraise Item / Item Tinkering
	CalcSkill(0x0013);	//Assess Person
	CalcSkill(0x0014);	//Deception
	CalcSkill(0x0015);	//Healing
	CalcSkill(0x0017);	//Lockpick
	CalcSkill(0x001B);	//Assess Creature
	CalcSkill(0x001C);	//Appraise Weapon / Weapon Tinkering
	CalcSkill(0x001D);	//Appraise Armor / Armor Tinkering
	CalcSkill(0x001E);	//Appraise Magic Item / Magic Item Tinkering
	CalcSkill(0x001F);	//Creature Enchantment
	CalcSkill(0x0020);	//Item Enchantment
	CalcSkill(0x0021);	//Life Magic
	CalcSkill(0x0022);	//War Magic
	CalcSkill(0x0025);	//Fletching
	CalcSkill(0x0026);	//Alchemy
	CalcSkill(0x0027);	//Cooking

	return cmUpdFocus;
}
/**
 *	Handles the message sent when an avatar's Self attribute is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Self. 
 *
 *	@return cMessage - Returns an Update Attribute (0x00000241) server message.
 */
cMessage cAvatar::UpdateSelf(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcAttributes[5].m_dwIncrement++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET Self = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcAttributes[5].m_dwIncrement,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	self_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Slf_exp_spent = %i WHERE AvatarGUID = %lu;",self_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Self message
	cMessage cmUpdSelf;
	cmUpdSelf << 0x241L << 0x06L << this->m_cStats.m_lpcAttributes[5].m_dwIncrement << this->m_cStats.m_lpcAttributes[5].m_dwCurrent << self_exp_cost;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Self is now %d!",this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement);

	//Skills affected by Self
	CalcSkill(0x000F);	//Magic Defense
	CalcSkill(0x0010);	//Mana Conversion
	CalcSkill(0x0013);	//Assess Person
	CalcSkill(0x0014);	//Deception
	CalcSkill(0x0017);	//Lockpick
	CalcSkill(0x001B);	//Assess Creature
	CalcSkill(0x001F);	//Creature Enchantment
	CalcSkill(0x0020);	//Item Enchantment
	CalcSkill(0x0021);	//Life Magic
	CalcSkill(0x0022);	//War Magic

	//Vitals affected by Self
	CalcVital(0x0002);	//Mana

	return cmUpdSelf;
}
	
//////////////////////////////////////
//	Vital updates		
//////////////////////////////////////

/**
 *	Calculates the total base value of a vital (Health, Stamina, Mana).
 *
 *	The base value will be equal to the creation valu plus increments to the vital.
 *	The resultant value is used for server-side calculations; the client maintains its own value (which should match the server's value).
 *
 *	@param vitalID - The numeric representation of the skill.
 */
void cAvatar::CalcVital(WORD vitalID)
{
	int vitalArrayNum = vitalID;
	
	int total_vital;

	total_vital = this->m_cStats.m_lpcVitals[vitalArrayNum].m_dwIncreases;

	//Add the vital bonuses
	switch(vitalID)
	{
		//The client rounds the result of attribute contributions to the vital level.
		//Given that high accuracy is not required, flooring the value plus one-half is precise enough.

		case 0x0000:	//Health
			{
				total_vital += floor((double)(this->base_endurance + this->m_cStats.m_lpcAttributes[1].m_dwIncrement) / 2 + .5);
				base_health = total_vital;
			}
			break;
		case 0x0001:	//Stamina
			{
				total_vital += this->base_endurance + this->m_cStats.m_lpcAttributes[1].m_dwIncrement;
				base_stamina = total_vital;
			}
			break;
		case 0x0002:	//Mana
			{
				total_vital += this->base_self + this->m_cStats.m_lpcAttributes[5].m_dwIncrement;
				base_mana = total_vital;
			}
			break;
	}

	this->m_cStats.m_lpcVitals[vitalArrayNum].m_dwCurrent = total_vital;
}

/**
 *	Handles the message sent when an avatar's Health vital is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Health. 
 *
 *	@return cMessage - Returns an Update Secondary Attribute (0x00000243) server message.
 */
cMessage cAvatar::RaiseHealth(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcVitals[0].m_dwIncreases++;
	this->m_cStats.m_lpcVitals[0].m_dwCurrent++;
	base_health++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET HealthInc = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcVitals[0].m_dwIncreases,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	health_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Hlt_exp_spent = %i WHERE AvatarGUID = %lu;",health_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Health message
	cMessage cmRaiseHealth;
	cmRaiseHealth << 0x243L << 0x01L << this->m_cStats.m_lpcVitals[0].m_dwIncreases << 0L << health_exp_cost << this->m_cStats.m_lpcVitals[0].m_lTrueCurrent;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Maximum Health is now %d!",this->base_health);
	
	return cmRaiseHealth;
}
/**
 *	Handles the message sent when an avatar's Stamina vital is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Stamina. 
 *
 *	@return cMessage - Returns an Update Secondary Attribute (0x00000243) server message.
 */
cMessage cAvatar::RaiseStamina(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcVitals[1].m_dwIncreases++;
	this->m_cStats.m_lpcVitals[1].m_dwCurrent++;
	base_stamina++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET StaminaInc = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcVitals[1].m_dwIncreases,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	stamina_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Sta_exp_spent = %i WHERE AvatarGUID = %lu;",stamina_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Stamina message
	cMessage cmRaiseStamina;
	cmRaiseStamina << 0x243L << 0x03L << this->m_cStats.m_lpcVitals[1].m_dwIncreases << 0L << stamina_exp_cost << this->m_cStats.m_lpcVitals[1].m_lTrueCurrent;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Maximum Stamina is now %d!",this->base_stamina);
	
	return cmRaiseStamina;
}
/**
 *	Handles the message sent when an avatar's Mana vital is incremented.
 *	Also updates the database value.
 *
 *	@param exp - The amount of experience spent incrementing Mana. 
 *
 *	@return cMessage - Returns an Update Secondary Attribute (0x00000243) server message.
 */
cMessage cAvatar::RaiseMana(DWORD exp)
{
	char		szCommand[512];
	RETCODE		retcode;

	//When this message is fired, the "Raise Stat" button has already been pressed.
	//The client will not let you click the "Raise" button if you don't have enough experience.
	this->m_cStats.m_lpcVitals[2].m_dwIncreases++;
	this->m_cStats.m_lpcVitals[2].m_dwCurrent++;
	base_mana++;

	//Add the increase to the database
	sprintf( szCommand, "UPDATE avatar SET ManaInc = %i WHERE AvatarGUID = %lu;",this->m_cStats.m_lpcVitals[2].m_dwIncreases,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//Set the avatar's experience cost equal to that of exp_cost + exp sent in the game event message.
	mana_exp_cost += exp;

	//Update database with the new total experience spent
	sprintf( szCommand, "UPDATE avatar SET Man_exp_spent = %i WHERE AvatarGUID = %lu;",mana_exp_cost,this->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	//The Update Mana message
	cMessage cmRaiseMana;
	cmRaiseMana << 0x243L << 0x05L << this->m_cStats.m_lpcVitals[2].m_dwIncreases << 0L << mana_exp_cost << this->m_cStats.m_lpcVitals[2].m_lTrueCurrent;
	cMessage cSound = this->SoundEffect(118,1.0);
	cWorldManager::SendToAllInFocus( this->m_Location, cSound, 3 );
	cMasterServer::ServerMessage(ColorBlue,NULL,"Your base Maximum Mana is now %d!",this->base_mana);
	
	return cmRaiseMana;
}

/**
 *	Handles the message sent when an avatar's Health vital is decremented.
 *
 *	@param amount - The amount of Health to decrement. 
 *	@param &newHealth - The address of an integer that will represent the avatar's new Health
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::DecrementHealth(WORD amount,signed int &newhealth)
{
	m_cStats.m_lpcVitals[0].m_lTrueCurrent -= amount;
	if (m_cStats.m_lpcVitals[0].m_lTrueCurrent < 0)
		m_cStats.m_lpcVitals[0].m_lTrueCurrent = 0;
	
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	if (aFellowship)
	{
		aFellowship->RelayMemberUpdate(GetGUID());
	}

	newhealth = m_cStats.m_lpcVitals[0].m_lTrueCurrent;
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x2L << static_cast<DWORD> (m_cStats.m_lpcVitals[0].m_lTrueCurrent);// << 0x3CL;
	return cmRet;
}
/**
 *	Handles the message sent when an avatar's Stamina vital is decremented.
 *
 *	@param amount - The amount of Stamina to decrement. 
 *	@param &newstamina - The address of an integer that will represent the avatar's new Stamina
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::DecrementStamina(WORD amount,signed int &newstamina)
{
	m_cStats.m_lpcVitals[1].m_lTrueCurrent -= amount;
	if (m_cStats.m_lpcVitals[1].m_lTrueCurrent < 0)
		m_cStats.m_lpcVitals[1].m_lTrueCurrent = 0;
	
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	if (aFellowship)
	{
		aFellowship->RelayMemberUpdate(GetGUID());
	}

	newstamina = m_cStats.m_lpcVitals[1].m_lTrueCurrent;
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x4L << static_cast<DWORD> (m_cStats.m_lpcVitals[1].m_lTrueCurrent);// << 0x3CL;
	return cmRet;
}
/**
 *	Handles the message sent when an avatar's Mana vital is decremented.
 *
 *	@param amount - The amount of Mana to decrement. 
 *	@param &newmana - The address of an integer that will represent the avatar's new Mana
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::DecrementMana(WORD amount,signed int &newmana)
{
	m_cStats.m_lpcVitals[2].m_lTrueCurrent -= amount;
	if (m_cStats.m_lpcVitals[2].m_lTrueCurrent < 0)
		m_cStats.m_lpcVitals[2].m_lTrueCurrent = 0;
	
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	if (aFellowship)
	{
		aFellowship->RelayMemberUpdate(GetGUID());
	}

	newmana = m_cStats.m_lpcVitals[2].m_lTrueCurrent;
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x6L << static_cast<DWORD> (m_cStats.m_lpcVitals[2].m_lTrueCurrent);// << 0x3CL;
	return cmRet;
}

/**
 *	Handles the message sent when an avatar's Health vital is changed.
 *
 *	@param dwNewHealth - The new Health value. 
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::SetHealth(DWORD dwNewHealth)
{
	m_cStats.m_lpcVitals[0].m_lTrueCurrent = static_cast<DWORD>(dwNewHealth);
	
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	if (aFellowship)
	{
		aFellowship->RelayMemberUpdate(GetGUID());
	}

	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x2L << dwNewHealth;
	return cmRet;
}
/**
 *	Handles the message sent when an avatar's Stamina vital is changed.
 *
 *	@param dwNewStamina - The new Stamina value. 
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::SetStamina(DWORD dwNewStamina)
{
	m_cStats.m_lpcVitals[1].m_lTrueCurrent = static_cast<DWORD>(dwNewStamina);
	
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	if (aFellowship)
	{
		aFellowship->RelayMemberUpdate(GetGUID());
	}

	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x4L << dwNewStamina;
	return cmRet;
}
/**
 *	Handles the message sent when an avatar's Mana vital is changed.
 *
 *	@param dwNewMana - The new Mana value. 
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::SetMana(DWORD dwNewMana)
{
	m_cStats.m_lpcVitals[2].m_lTrueCurrent = static_cast<DWORD>(dwNewMana);
	
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	if (aFellowship)
	{
		aFellowship->RelayMemberUpdate(GetGUID());
	}

	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x6L << dwNewMana;
	return cmRet;
}

/**
 *	Handles the message sent when an avatar's Health vital is updated by an item (food, drink, potion, etc).
 *
 *	@param amount - The amount by which the avatar's Health should increase. 
 *	@param &newvalue - The address of an integer that will represent the avatar's new Health
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::UpdateHealth(WORD amount,signed int &newvalue )
{
	m_cStats.m_lpcVitals[0].m_lTrueCurrent += amount;
	if (m_cStats.m_lpcVitals[0].m_lTrueCurrent > m_cStats.m_lpcVitals[0].m_dwCurrent)
		m_cStats.m_lpcVitals[0].m_lTrueCurrent = m_cStats.m_lpcVitals[0].m_dwCurrent;
	newvalue = m_cStats.m_lpcVitals[0].m_lTrueCurrent;
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x2L << static_cast<DWORD> (m_cStats.m_lpcVitals[0].m_lTrueCurrent);
	return cmRet;
}
/**
 *	Handles the message sent when an avatar's Stamina vital is updated by an item (food, drink, potion, etc).
 *
 *	@param amount - The amount by which the avatar's Stamina should increase. 
 *	@param &newvalue - The address of an integer that will represent the avatar's new Stamina
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::UpdateStamina(WORD amount,signed int &newvalue )
{
	m_cStats.m_lpcVitals[1].m_lTrueCurrent += amount;
	if (m_cStats.m_lpcVitals[1].m_lTrueCurrent > m_cStats.m_lpcVitals[1].m_dwCurrent)
		m_cStats.m_lpcVitals[1].m_lTrueCurrent = m_cStats.m_lpcVitals[1].m_dwCurrent;
	newvalue = m_cStats.m_lpcVitals[1].m_lTrueCurrent;
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x4L << static_cast<DWORD> (m_cStats.m_lpcVitals[1].m_lTrueCurrent);
	return cmRet;
}
/**
 *	Handles the message sent when an avatar's Mana vital is updated by an item (food, drink, potion, etc).
 *
 *	@param amount - The amount by which the avatar's Mana should increase. 
 *	@param &newvalue - The address of an integer that will represent the avatar's new Mana
 *
 *	@return cMessage - Returns an Update Vital Statistic (0x00000244) server message.
 */
cMessage cAvatar::UpdateMana(WORD amount,signed int &newvalue )
{
	m_cStats.m_lpcVitals[2].m_lTrueCurrent += amount;
	if (m_cStats.m_lpcVitals[2].m_lTrueCurrent > m_cStats.m_lpcVitals[2].m_dwCurrent)
		m_cStats.m_lpcVitals[2].m_lTrueCurrent = m_cStats.m_lpcVitals[2].m_dwCurrent;
	newvalue = m_cStats.m_lpcVitals[2].m_lTrueCurrent;
	cMessage cmRet;
	cmRet << 0x244L << m_bStatSequence++ << 0x6L << static_cast<DWORD> (m_cStats.m_lpcVitals[2].m_lTrueCurrent);
	return cmRet;
}

cMessage cAvatar::AdjustHealthBar(DWORD dwGUID, DWORD F7B0Sequence )
{
	cMessage cmHealthLoss;
	double flHealthStat = ((( double )100/ m_cStats.m_lpcVitals[0].m_dwCurrent)* m_cStats.m_lpcVitals[0].m_lTrueCurrent)/100;
	
	cmHealthLoss << 0xF7B0L << dwGUID << F7B0Sequence << 0x01C0L << m_dwGUID << float(flHealthStat);
	return cmHealthLoss;
}

/**
 *	Updates the avatar's level.
 *
 *	The level is calculated based upon a formula that uses the avatar's total experience.
 *	Whether to award a skill credit is determined based upon pre-defined table entries.
 *
 *	The resultant value is used for server-side calculations; the client maintains its own value (which should match the server's value).
 */
cMessage cAvatar::UpdateLevel()
{
	cMessage cmRet;

	//The level formula; calculates the level that corresponds to the current total experience
	int level = floor( ( pow( (double)(9 * m_dwTotalXP + 6^5),(double)(1 / 5.0) ) - 6) + 1 );

	while (this->m_cStats.m_dwLevel < level && this->m_cStats.m_dwLevel < MAX_LEVEL)
	{
		this->m_cStats.m_dwLevel++;

		int nextSkillCreditLevel;
		char szCommand [200];
		RETCODE retcode;

		//Determine whether a skill credit will be received
		sprintf( szCommand, "SELECT skill_credits FROM exp_table WHERE ID = %d;",this->m_bTotalSkillCredits + 1);
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &nextSkillCreditLevel, sizeof( nextSkillCreditLevel ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFetch( cDatabase::m_hStmt );
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	

		if (this->m_cStats.m_dwLevel == nextSkillCreditLevel)
		{
			this->m_bSkillCredits++;
			this->m_bTotalSkillCredits++;
		}
		nextSkillCreditLevel = 0;

		//Determine at what level the next skill credit (if any) will be received
		sprintf( szCommand, "SELECT skill_credits FROM exp_table WHERE ID = %d;",this->m_bTotalSkillCredits + 1);
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &nextSkillCreditLevel, sizeof( nextSkillCreditLevel ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFetch( cDatabase::m_hStmt );
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	

		if (this->m_cStats.m_dwLevel == MAX_LEVEL)
		{
			cMessage cParticle = this->Particle(140);
			cWorldManager::SendToAllInFocus( this->m_Location, cParticle, 3 );
		
			cMasterServer::ServerMessage(ColorBlue,NULL,"You have reached the maximum level of %d!",MAX_LEVEL);
		} else {
			cMessage cParticle = this->Particle(137);
			cWorldManager::SendToAllInFocus( this->m_Location, cParticle, 3 );

			cMasterServer::ServerMessage(ColorBlue,NULL,"Congratulations, you are now level %d!  You have %d skill credits and will gain another at level %d.",this->m_cStats.m_dwLevel,this->m_bSkillCredits,nextSkillCreditLevel);
		}
	}
	return cmRet;	
}

/**
 *	Handles the message sent for the avatar's animation when running towards a combat target.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::RunToAnimation(DWORD dwGUID,DWORD dwTarget)
{
	cMessage cmRet;
		cmRet << 0xF74CL 
			  << &dwGUID 	 // GUID
			  << WORD(0x3C)  // numLogins
			  << WORD(0x19)  // Seq
			  << WORD(0x00)  // numAnimations
			  << WORD(0x01)  // activity 0x0 idle 0x1 active
			  << WORD(0x00)  // Unkown
			  << 0x00000081  // Animation Flags
			  << WORD(0x07)  // 
			  << WORD(0x07)  // Run
			  << WORD(0x3D) 	 // Animation2
			  << 2.0f;	 // numAnimations
			  //<< ;
	return cmRet;
}

/**
 *	Handles the message sent for the avatar's animation when turning towards a combat target.
 *
 *	@return cMessage - Returns an Animation (0x0000F74C) server message.
 */
cMessage cAvatar::TurnToTarget( float flHeading, DWORD dwTargetGUID )
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
					<< WORD(0x0000) //stance: 0x0000 to finish with previous stance
					<< dwTargetGUID
					<< flHeading
					<< 0x1231EE0F //0x1139EE0F - old value, 0x00000000 doesn't end the cast animation
					<< 0x3F800000 //1.0f
					<< 0x00000000;

	return cmTurnToTarget;
}

// End Combat Routines

//////////////////////////////////////
//	Fellowship Commands				
//////////////////////////////////////
/**
 *	Handles the creation of a fellowship.
 *
 *	@param F7B0Sequence - The client's present F7B0 sequence value.
 *	@param strFellowName - The fellowship's name.
 */
void cAvatar::CreateFellowship( DWORD F7B0Sequence, char strFellowName[50] )
{
	cClient* pcClient = cClient::FindClient(m_dwGUID);

/*
Fellowship Settings:
	0x00000008 = Accept Fellowship Requests					uncheckmarked
	0x00040000 = Share Fellowship Experience				checkmarked
	0x00100000 = Share Fellowship Loot/Treasure				checkmarked
	0x20000000 = Auto-Accept Fellowship Requests			checkmarked
*/

	bool shareXP, shareLoot;
	(this->m_dwOptions & 0x00040000) ? shareXP = true : shareXP = false;
	(this->m_dwOptions & 0x00100000) ? shareLoot = true : shareLoot = false;

	SetFellowshipID(cFellowship::NewFellowship(this->GetGUID(), this->Name(), strFellowName, shareXP, shareLoot));
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());

	if (aFellowship)
	{
		this->inFellow = true;
	}
}		

/**
 *	Handles the quitting from or disbanding of a fellowship.
 */	
void cAvatar::DisbandFellowship( )
{
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());
	aFellowship->RemMember(GetGUID());

	SetFellowshipID(0);
	this->inFellow = false;
}

/**
 *	Encapsulates messages sent to sender after an offer of fellowship is accepted or declined.
 *
 *	@param szTargetName - The target character's name.
 *	@param dwReply - The reply (yes/no) to the offer of fellowship.
 *	@param dwReceiptGUID - The target character's GUID.
 */	
void cAvatar::FellowshipRecruitSend(std::string szTargetName, DWORD dwReply, DWORD dwReceiptGUID)
{
	cClient* pcClient = cClient::FindClient(m_dwGUID);
	cClient* pcRecvClient = cClient::FindClient(dwReceiptGUID);

	if (pcRecvClient->m_pcAvatar)
	{
		cMessage	cmRecruitText;
		char		szTextBuffer[255];
		DWORD		dwColor = 0x17L;

		if (dwReply == 1)
		{
			sprintf(&szTextBuffer[0],"%s is now a member of your Fellowship.", pcRecvClient->m_pcAvatar->Name());
		}
		else
		{
			sprintf(&szTextBuffer[0],"%s has declined your offer of fellowship.", pcRecvClient->m_pcAvatar->Name());
		}

		cmRecruitText << 0xF62CL << szTextBuffer << dwColor;
		pcClient->AddPacket( WORLD_SERVER, cmRecruitText, 4 );
	}
}

/**
 *	Encapsulates messages sent to receiver after an offer of fellowship is accepted or declined.
 *
 *	@param szTargetName - The sending character's name.
 *	@param dwReply - The reply (yes/no) to the offer of fellowship.
 *	@param dwSenderGUID - The sending character's GUID.
 */	
void cAvatar::FellowshipRecruitRecv(std::string szTargetName, DWORD dwReply, DWORD dwSenderGUID)
{
	cClient* pcClient = cClient::FindClient(m_dwGUID);
	cClient* pcSendClient = cClient::FindClient(dwSenderGUID);

	if (pcSendClient->m_pcAvatar)
	{
		if (dwReply == 1)
		{
			// add the avatar to the fellowship
			cAvatar* pcRecruitAvatar = pcSendClient->m_pcAvatar;
			cFellowship* aFellowship = cFellowship::GetFellowshipByID(pcRecruitAvatar->m_dwFellowshipID);
			aFellowship->AddMember(pcClient->m_pcAvatar->GetGUID());
			
			this->inFellow = true;
			SetFellowshipID(pcRecruitAvatar->m_dwFellowshipID);

			cMessage	cmRecruitText;
			char		szTextBuffer[255];
			DWORD dwColor = 0x17L;
			sprintf(&szTextBuffer[0],"You have been recruited into the %s fellowship, a fellowship led by %s.", aFellowship->GetName().c_str(), pcSendClient->m_pcAvatar->Name());
			cmRecruitText << 0xF62CL << szTextBuffer << dwColor;
			pcClient->AddPacket( WORLD_SERVER, cmRecruitText, 4 );
		}
		else
		{
		}
	}
}

/**
 *	Removes a character from the fellowship and relays the relevant messages.
 *
 *	@param dwMemberGUID - The GUID of the character to remove.
 */	
void cAvatar::FellowshipDismiss( DWORD dwMemberGUID )
{
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());

	if (aFellowship)
	{
		if (aFellowship->m_LeaderGUID == this->GetGUID())
		{
			aFellowship->DismissMember(dwMemberGUID);
		}
	}
}

/**
 *	Sets a fellowship open or closed and relays the relevant messages.
 *
 *	@param dwOpen - Determines whether the fellowship is opened or closed (0 = closed; 1 = opened).
 */	
void cAvatar::OpenCloseFellowship( DWORD dwOpen )
{
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());

	if (aFellowship)
	{
		if (aFellowship->m_LeaderGUID == this->GetGUID())
		{
			aFellowship->SetOpenClose(dwOpen);
		}
	}
}

/**
 *	Promotes a character to fellowship leader and relays the relevant messages.
 *
 *	@param dwNewLeaderGUID - The GUID of the character to promote.
 */	
void cAvatar::FellowshipLeader( DWORD dwNewLeaderGUID )
{
	cFellowship* aFellowship = cFellowship::GetFellowshipByID(GetFellowshipID());

	if (aFellowship)
	{
		if (aFellowship->m_LeaderGUID == this->GetGUID())
		{
			aFellowship->SetLeader(dwNewLeaderGUID);
		}
	}
}

/**
 *	Handles the message sent for an avatar's housing information.
 *	Also updates the respective database entries.
 *
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type House Information for Non-Owners (0x00000226).
 */
cMessage cAvatar::HouseAbandon( DWORD F7B0seq )
{
	cMessage cmReturn;

	cmReturn	<< 0xF7B0L
				<< GetGUID( )
				<< F7B0seq;		// sequence number

	if(!m_wHouseID)
	{
		//cMasterServer::ServerMessage(ColorGreen,this,"You must own a house to use this command!");							
	} else {
		char	szCommand[250];
		RETCODE	retcode;

		char	CovGUIDBuf[9];
		DWORD	CovGUID;

		sprintf( szCommand, "SELECT GUID from houses_covenants WHERE HouseID = %d;",m_wHouseID );
																												
		retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, &CovGUIDBuf, sizeof( CovGUIDBuf ), NULL );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
												
		if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
		{
			sscanf(CovGUIDBuf,"%08x",&CovGUID);
		}
		
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )			
			
		sprintf( szCommand, "UPDATE houses_covenants SET OwnerID = %d WHERE GUID = '%08x';",0,CovGUID );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

		UpdateConsole (szCommand);
		sprintf( szCommand, "UPDATE houses SET OwnerID = %d WHERE wModel = %d;",0,m_wHouseID );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												
		sprintf( szCommand, "UPDATE houses_purchase SET Paid = %d WHERE wModel = %d;",0,m_wHouseID );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)		

		m_wHouseID = NULL;

		cmReturn << 0x226L;
	
		cMasterServer::ServerMessage(ColorGreen,NULL,"You abandon your house!");
	}
	
	return cmReturn;
}
/**
 *	Adds a guest to the avatar's house guest list database entries.
 *
 *	@param strGuestName - The name of the guest to add.
 */
void cAvatar::HouseGuestAdd( char strGuestName[50] )
{
	if (m_wHouseID)
	{
		char	szCommand[512];
		RETCODE	retcode;

		UINT	dwGuestGUID;
		WORD	wGuestCount = 0;
		char	szPacket[60];

		sprintf( szCommand, "SELECT AvatarGUID FROM avatar WHERE Name = '%s';",strGuestName );										
		
		retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwGuestGUID, sizeof( dwGuestGUID ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLFetch( cDatabase::m_hStmt );
		
		if( retcode == SQL_NO_DATA )
		{
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			cMasterServer::ServerMessage(ColorGreen,NULL,"That character does not exist.");
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				
			sprintf( szCommand, "SELECT COUNT(ID) FROM houses_guest_lists WHERE HouseID = %d AND GuestGUID = %d;",m_wHouseID,dwGuestGUID );
			
			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wGuestCount, sizeof( wGuestCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLFetch( cDatabase::m_hStmt );
												
			if( wGuestCount > 0)
			{
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												
				sprintf (szPacket, "%s is already on your guest list.",strGuestName);
				cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
			} else {
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												
				sprintf( szCommand, "INSERT INTO houses_guest_lists ( HouseID,GuestGUID,StorageAccess ) VALUES (%lu,%lu,%d);",m_wHouseID,dwGuestGUID,0 );
				retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
				retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
												
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

				sprintf (szPacket, "%s has been added to your guest list.",strGuestName);
				cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
			}							
		}
	}
}
/**
 *	Removes a guest from the avatar's house guest list database entries.
 *
 *	@param strGuestName - The name of the guest to remove.
 */
void cAvatar::HouseGuestRemoveName( char strGuestName[50] )
{
	if (m_wHouseID)
	{
		char	szCommand[512];
		RETCODE	retcode;

		UINT	dwGuestGUID;
		WORD	wGuestCount = 0;

		char	szPacket[60];

		sprintf( szCommand, "SELECT AvatarGUID FROM avatar WHERE Name = '%s';",strGuestName );										
										
		retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwGuestGUID, sizeof( dwGuestGUID ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLFetch( cDatabase::m_hStmt );
										
		if( retcode == SQL_NO_DATA )
		{
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			cMasterServer::ServerMessage(ColorGreen,NULL,"That character does not exist.");
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

			sprintf( szCommand, "SELECT COUNT(ID) FROM houses_guest_lists WHERE HouseID = %d AND GuestGUID = %lu;",m_wHouseID,dwGuestGUID );

			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &wGuestCount, sizeof( wGuestCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLFetch( cDatabase::m_hStmt );
												
			if( wGuestCount > 0)
			{
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				
				sprintf( szCommand, "DELETE FROM houses_guest_lists WHERE HouseID = %d AND GuestGUID = %lu;",m_wHouseID,dwGuestGUID );
				retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
				retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
													
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

				sprintf (szPacket, "You have removed %s from your guest list.",strGuestName);
				cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
			} else {
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												
				sprintf (szPacket, "%s is not on your guest list.",strGuestName);
				cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
			}								
		}
	}
}

/**
 *	The open/closed value of the avatar's house database entry.
 *
 *	@param dwIsOpen - A value representing whether the house is now open or closed.
 */
void cAvatar::HouseOpenClose( DWORD dwIsOpen )
{
	if (m_wHouseID)
	{
		char	szCommand[512];
		RETCODE	retcode;

		DWORD	dwWasOpen;

		sprintf( szCommand, "SELECT isOpen FROM houses WHERE wModel = %d;",m_wHouseID );
										
		retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwWasOpen, sizeof( dwWasOpen ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLFetch( cDatabase::m_hStmt );
												
		if( retcode == SQL_NO_DATA )
		{
			//cMasterServer::ServerMessage(ColorGreen,this,"You must own a house to use this command.");
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		} else {
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			
			if (dwWasOpen == 1 && dwIsOpen == 1)
			{
				cMasterServer::ServerMessage(ColorGreen,NULL,"You already have an open house.");
			} else if (dwWasOpen == 0 && dwIsOpen == 0) {
				cMasterServer::ServerMessage(ColorGreen,NULL,"You already have a closed house.");
			} else {
				sprintf( szCommand, "UPDATE houses SET isOpen = %d WHERE OwnerID = (%d);",dwIsOpen, GetGUID( ) );
				retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
				retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
				if (dwIsOpen == 1)
				{
					cMasterServer::ServerMessage(ColorGreen,NULL,"Your house is now open to the public.");
				} else {
					cMasterServer::ServerMessage(ColorGreen,NULL,"Your house is now closed to the public.");
				}
			}
		}
	}
}

/**
 *	Updates the storage permission for a guess on the avatar's house guest list database entries.
 *
 *	@param strGuestName - The name of the guest to whose permissions should be modified.
 *	@param dwStorageSet - A value representing whether access to storage is permitted or revoked.
 */
void cAvatar::HouseStorage( char strGuestName[50], DWORD dwStorageSet )
{
	if (m_wHouseID)
	{
		char	szCommand[512];
		RETCODE	retcode;

		char	GuestIDBuff[9];
		DWORD	GuestID;
		WORD	wGuestCount = 0;
		DWORD	StorageOpen;

		char	szPacket[60];

		sprintf( szCommand, "SELECT AvatarGUID FROM avatar WHERE Name = '%s';",strGuestName );										
											
		retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, GuestIDBuff, sizeof( GuestIDBuff ), NULL );		CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLFetch( cDatabase::m_hStmt );
											
		if( retcode == SQL_NO_DATA )
		{
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			cMasterServer::ServerMessage(ColorGreen,NULL,"That character does not exist.");
		} else {
			sscanf(GuestIDBuff,"%08x",&GuestID);
			retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
			retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

			sprintf( szCommand, "SELECT StorageAccess FROM houses_guest_lists WHERE HouseID = %d AND GuestGUID = %d;",m_wHouseID,GuestID );

			retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &StorageOpen, sizeof( StorageOpen ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
			retcode = SQLFetch( cDatabase::m_hStmt );
													
			if( retcode == SQL_NO_DATA )
			{
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

				sprintf (szPacket, "%s must be on your guest list to be given access to your home's storage.",strGuestName);
				cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
			} else {
				retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
				retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
				
				if(dwStorageSet == 1)
				{
					if (StorageOpen == 1)
					{
						sprintf (szPacket, "%s has already been given access to your home's storage.",strGuestName);
						cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
					} else {
						sprintf( szCommand, "UPDATE houses_guest_lists SET StorageAccess = %d WHERE HouseID = %d AND GuestGUID = %d;",1,m_wHouseID,GuestID );
						retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
						retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
						retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );								CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

						sprintf (szPacket, "You have granted %s access to your home's storage.  This is denoted by the asterisk next to their name in the guest list.",strGuestName);
						cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
					}
				} else {
					if (StorageOpen == 1)
					{
						sprintf( szCommand, "UPDATE houses_guest_lists SET StorageAccess = %d WHERE HouseID = %d AND GuestGUID = %d;",0,m_wHouseID,GuestID );
						retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
						retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
						retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );								CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

						sprintf (szPacket, "You have removed access to your home's storage from %s.  This is denoted by the asterisk next to their name in the guest list.",strGuestName);
						cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
					} else {
						sprintf (szPacket, "%s does not currently have access to your home's storage.",strGuestName);
						cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
					}
				}	
			}
		}
	}
}

void cAvatar::HouseBootName( char strGuestName[50] ) { }

/**
 *	Removes all guests on the avatar's house guest list database entries.
 */
void cAvatar::HouseStorageRemoveAll( )
{
	if (m_wHouseID)
	{
		char	szCommand[512];
		RETCODE	retcode;
		char	szPacket[60];

		sprintf( szCommand, "UPDATE houses_guest_lists SET StorageAccess = %d WHERE HouseID = %d;",0,m_wHouseID );
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
		retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );		CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
											
		sprintf (szPacket, "You remove item storage permission from all your guests.");
		cMasterServer::ServerMessage(ColorGreen,NULL,(char *)szPacket);
		retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )		
	}
}

/**
 *	Handles the message sent for an avatar's housing guest list.
 *
 *	@return cMessage - Returns a Game Event (0x0000F7B0) server message of type House Guest List (0x00000257).
 */
cMessage cAvatar::HouseGuestList( DWORD F7B0seq )
{
	cMessage cmReturn;

	cmReturn << 0xF7B0L << GetGUID( ) << F7B0seq << 0x0257L;
										
	char	szCommand[250];
	RETCODE	retcode;

	WORD			IsOpen;
	WORD			AllegPerm = 0;
	WORD			wGuestCount = 0;
	UINT			dwGuestGUID;
	DWORD			StorageOpen;
	char			strGuestName[50];
	std::string		strName = "";

	sprintf( szCommand, "SELECT isOpen FROM houses WHERE wModel = %d;",m_wHouseID );
										
	//sprintf( szCommand, "SELECT OwnerID,wModel,isOpen FROM houses WHERE OwnerID = %d;",GetGUID( ) );
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
										
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_SHORT, &IsOpen, sizeof( IsOpen ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLFetch( cDatabase::m_hStmt );

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	//cMasterServer::ServerMessage(ColorGreen,this,"Guest List:");

	sprintf( szCommand, "SELECT COUNT(ID) FROM houses_guest_lists WHERE HouseID=%d;",m_wHouseID );
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
											
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_SHORT, &wGuestCount, sizeof( wGuestCount ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLFetch( cDatabase::m_hStmt );
																					
	if( retcode == SQL_NO_DATA ) { wGuestCount = 0; }

	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

	BYTE houseBYTE = 0x00;
	if (IsOpen == 1)
	{
		houseBYTE = houseBYTE | BYTE(0x01);
	}
	// Dwelling access control list
	cmReturn	<< 0x10000002	// DWORD flags -- believed to be flags that control the size and content of this structure
				<< houseBYTE	// xxx1 = open house; xx1x = allegiance guest; x1xx = allegiance storage
				<< BYTE(0x00)	// WORD open:  0 = private dwelling, 1 = open to public
				<< WORD(0x0000)	// unknown
				<< 0x00000000	// ObjectID allegiance -- allegiance monarch (if allegiance access granted)
				<< wGuestCount	// WORD guestCount -- number of guests on list
				<< WORD(0x0300);// WORD guestLimit -- Maximum number of guests on guest list (cottage is 32)

	//sprintf( szCommand, "SELECT GuestGUID,StorageAccess FROM houses_guest_lists WHERE HouseID=%d;",HouseID );										
	sprintf( szCommand, "SELECT houses_guest_lists.GuestGUID,houses_guest_lists.StorageAccess,avatar.Name FROM {oj houses_guest_lists LEFT OUTER JOIN avatar ON houses_guest_lists.GuestGUID=avatar.AvatarGUID} WHERE houses_guest_lists.HouseID=%d;",m_wHouseID );										
											
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
												
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwGuestGUID, sizeof( dwGuestGUID ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &StorageOpen, sizeof( WORD ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_CHAR, strGuestName, sizeof( strGuestName ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
	{
		cmReturn << DWORD(dwGuestGUID) << StorageOpen;	//0x0000 = access to dwelling; 0x0001 = access also to storage;
		strName.assign(strGuestName);
		cmReturn << strName.c_str();	// Guest's Name
	}
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	cmReturn << 0x00000000;				// Number of GUIDs that follow?
	//cmGuestList << m_pcAvatar->GetGUID( );//Unknown GUID
	//if (IsOpen) { cMasterServer::ServerMessage(ColorGreen,this,"[ open house ]"); }

	return cmReturn;
}

DWORD cAvatar::CalculateDamage( cObject *pcWeapon, float flPower, float flResistance )
{	
	srand( timeGetTime( ) );

	int iRand = rand( );
	iRand = (iRand > 6400) ? iRand % 6400 : iRand;
	iRand = (iRand < 1600) ? 1600 + iRand : iRand;
	
	double dLuckFactor = 3200.0f / (double)iRand; 

	// MaxDamage = (Strength * 0.2 * (flPower + 1.15)) / (flResistance + 5.0)

	// This is the old formula
	double dMaxDamage = (double)((m_cStats.m_lpcAttributes[0].m_dwCurrent - 55 * 0.11f) * ((flPower + .25f) * 4)) / (double)(flResistance + 7.5f);
	
	// Cubem0j0: New formula
	//double dMaxDamage = 0.11f * (m_cStats.m_lpcAttributes[0].m_dwCurrent-55)+1;
	return dMaxDamage * dLuckFactor + (rand( ) % 2);
}

void cAvatar::CreateChildren( cClient *pcClientDestination )
{
	/*cMessage cmReturn;

	fGotObject = FALSE;

	static iterObject_lst itObject = m_lstInventory.begin( );
	iterObject_lst itObjectReturn;

	if( m_lstInventory.size() > 0 )
	{
		while ( ( itObject != m_lstInventory.end( ) ) && ( !fGotObject ) )
		{
			if( ( *itObject )->m_fEquipped )
			{
				itObjectReturn = itObject;
				++itObject;
				fGotObject = TRUE;
			}
			else
				++itObject;

		}
		
		if( fGotObject )
			return ( *itObjectReturn )->CreatePacket( );
		else
			itObject = m_lstInventory.begin( );
		return cmReturn;
	}
	else
	{
		itObject = m_lstInventory.begin( );
		return cmReturn;
	}*/

	for ( iterObject_lst itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
		if ( ( *itObject )->m_fEquipped == 1)
			pcClientDestination->AddPacket( WORLD_SERVER, ( *itObject )->CreatePacket( ), 3 );
}

float cAvatar::GetRange ( DWORD dwLandblock, float flX, float flY, float flZ, DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ )
{
	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float intRange;

	  nsCoord = (((((dwLandblock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(flY / 24) - 1027.5; 
      ewCoord = (((((dwLandblock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(flX / 24) - 1027.5;
	  nsTarCoord = (((((dwTargetLandblock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(flTarY / 24) - 1027.5; 
      ewTarCoord = (((((dwTargetLandblock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(flTarX / 24) - 1027.5;
	
	  intRange = sqrt(pow(nsTarCoord - nsCoord,2) + pow(ewTarCoord - ewCoord,2));

	 return intRange;

}

float cAvatar::GetHeadingTarget( DWORD dwLandblock, float flX, float flY, float flZ, DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ )
{

	float nsCoord,ewCoord;
	float nsTarCoord,ewTarCoord;
	float flHeading;
	float intRange;

	flHeading = 0.0f;

	nsCoord = (((((dwLandblock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(flY / 24) - 1027.5; 
	ewCoord = (((((dwLandblock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(flX / 24) - 1027.5;
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
	return flHeading;
}

/**
 *	Handles the assessment of avatars.
 *
 *	This function is called whenever an avatar is assessed by another client.
 *	Returns a Game Event (0x0000F7B0) server message of type Identify Object (0x000000C9).
 */
void cAvatar::Assess(cClient *pcAssesser)
{
	cMessage cmAssess;
	WORD wAnimation;
	wAnimation = 0x54L;

	std::string monarchName = "";
	std::string patronName = "";
	DWORD playerRank = 0xFFFFFFFF;
	DWORD monarchFollowers = 0xFFFFFFFF;
	if (cAllegiance* aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID))
	{
		DWORD monarchGUID = DWORD(aAllegiance->GetLeader());						// Monarch's GUID
		Member monarchRecord = aAllegiance->members.find(monarchGUID)->second;		// Monarch's allegiance record
		monarchFollowers = monarchRecord.m_dwFollowers;								// Monarchs number of followers
		// Do not display the monarch name if the player is the monarch
		if (m_dwGUID != aAllegiance->GetLeader())
		{
			monarchName = aAllegiance->members.find(monarchGUID)->second.m_szName;	// Monarch's Name
		}

		Member playerRecord = aAllegiance->members.find(m_dwGUID)->second;			// Player's allegiance record
		playerRank = DWORD(playerRecord.m_bRank);									// Player's allegiance rank

		if (m_dwGUID != aAllegiance->GetLeader())
		{
			DWORD patronGUID = playerRecord.m_dwPatron;								// Patron's GUID
			patronName = aAllegiance->members.find(patronGUID)->second.m_szName;	// Patron's Name
		}
	}

	DWORD flags;
	DWORD dwCharacterFlags;			// Needs Decoding
	
	flags = 0x00000100L;			// Flags 0x00000100 - Character Data
	dwCharacterFlags = 0x0000000EL; // 0x04 - Level, Health | 0x08 - Stats | 0x02 - Miscellaneous

	cmAssess	<< 0xF7B0L 
				<< pcAssesser->m_pcAvatar->GetGUID( ) 
				<< ++pcAssesser->m_dwF7B0Sequence 
				<< 0x000000C9L 
				<< m_dwGUID
				<< flags	
				<< 0x1L;	// Success: 0 = False; 1 = True
	
	// Process the flag-specified data

	if (flags & 0x00000100)
	{
		cmAssess	<< 	dwCharacterFlags;

		if (dwCharacterFlags & 0x00000004)	// Mask 0x04
		{
			cmAssess	<< m_cStats.m_dwLevel						//Level (0 if assess fails)
						<< m_cStats.m_lpcVitals[0].m_lTrueCurrent	//Current Health
						<< m_cStats.m_lpcVitals[0].m_dwCurrent;		//Maximum Health
		}
		if (dwCharacterFlags & 0x00000008)	// Mask 0x08
		{
			cmAssess	<< m_cStats.m_lpcAttributes[0].m_dwCurrent + m_cStats.m_lpcAttributes[0].m_dwIncrement//m_cStats.m_dwStr
						<< m_cStats.m_lpcAttributes[1].m_dwCurrent + m_cStats.m_lpcAttributes[1].m_dwIncrement//m_cStats.m_dwEnd
						<< m_cStats.m_lpcAttributes[2].m_dwCurrent + m_cStats.m_lpcAttributes[2].m_dwIncrement//m_cStats.m_dwQuick
						<< m_cStats.m_lpcAttributes[3].m_dwCurrent + m_cStats.m_lpcAttributes[3].m_dwIncrement//m_cStats.m_dwCoord
						<< m_cStats.m_lpcAttributes[4].m_dwCurrent + m_cStats.m_lpcAttributes[4].m_dwIncrement//m_cStats.m_dwFocus
						<< m_cStats.m_lpcAttributes[5].m_dwCurrent + m_cStats.m_lpcAttributes[5].m_dwIncrement//m_cStats.m_dwSelf
						<< m_cStats.m_lpcVitals[1].m_lTrueCurrent	//m_dwCurrentStamina
						<< m_cStats.m_lpcVitals[2].m_lTrueCurrent	//m_cStats.m_dwCurrentMana
						<< m_cStats.m_lpcVitals[1].m_dwCurrent		//m_dwMaxStamina
						<< m_cStats.m_lpcVitals[2].m_dwCurrent		//m_dwMaxMana
						<< DWORD(m_wRace);
		}
		if (dwCharacterFlags & 0x00000002)	// Mask 0x02
		{
			cmAssess	<<	playerRank				//DWORD rank (0xFFFFFFFF for a character without a patron)
						<<	monarchFollowers		//DWORD followers (number of followers of an allegiance monarch; otherwise 0xFFFFFFFF)
						<<	m_cStats.m_lpcSkills[36].m_dwTotal		//DWORD loyalty (loyalty of characters without a patron; otherwise 0)
						<<	m_cStats.m_lpcSkills[35].m_dwTotal		//DWORD leadership (leadership of characters without a patron; otherwise 0)
						<<	DWORD(m_fIsPK)			//DWORD PK (Non-PK = 0; PK = 1)
						<<	m_strSexName			//String sex
						<<	m_strRaceName			//String race
						<<	m_strClassName			//String class
						<<	""						//String fellowship (empty string for none)
						<<	monarchName.c_str()		//String monarch (empty string for none)
						<<	patronName.c_str()		//String patron (empty string for none)
						<<	DWORD(m_BasicVectorTex[1].m_wNewTexture | 0x05000000)	//DWORD forehead texture
						<<	DWORD(m_BasicVectorTex[2].m_wNewTexture | 0x05000000)	//DWORD nose texture
						<<	DWORD(m_BasicVectorTex[3].m_wNewTexture | 0x05000000)	//DWORD chin texture
						<<	DWORD(m_BasicVectorPal[2].m_wNewPalette | 0x04000000)	//DWORD eyes palette
						<<	DWORD(m_BasicVectorPal[1].m_wNewPalette | 0x04000000)	//DWORD hair palette
						<<	DWORD(m_BasicVectorPal[0].m_wNewPalette | 0x04000000);	//DWORD skin palette
		}
		if (dwCharacterFlags & 0x00000001)	// Mask 0x01
		{
			cmAssess	<< 0x00000000;		//DWORD extra		
		}
	}
	if (flags & 0x00000800)
	{
		cmAssess	<< 0x00000000;		//DWORD highlights
	}

	pcAssesser->AddPacket(WORLD_SERVER,cmAssess,4);
	// the action has been completed
	cMessage cmActionComplete;
	cmActionComplete << 0xF7B0L << pcAssesser->m_pcAvatar->GetGUID( ) << ++pcAssesser->m_dwF7B0Sequence << 0x00C9L << 0L;
	pcAssesser->AddPacket(WORLD_SERVER,cmActionComplete,4);
}

void cAvatar::SetIsPK_DB()
{
	char szCommand[150];
	// Update User's Database IsPK Flag
	sprintf( szCommand, "UPDATE Avatar SET IsPK=%d WHERE AvatarGUID=%d;", m_fIsPK,m_dwGUID );

	RETCODE retcode;
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );								CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
}

//Cubem0j0:  Update quest table function.
void cAvatar::UpdateQuestCompletedTable(DWORD dwAvatarGUID, DWORD quest_id)
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime (buffer,80,"%X",timeinfo);

	//Update the avatar quest completed table to reflect that this avatar has completed this quest
	char szCommand[100];
	RETCODE retcode;

	//debug...
	#ifdef _DEBUG
	cMasterServer::ServerMessage( ColorYellow,NULL,"AvatarGUID is: %lu, quest_id is: %d",dwAvatarGUID,quest_id);
	#endif

	sprintf(szCommand,"INSERT INTO avatar_comp_quests (AvatarGUID,quest_id,completed) VALUES (%u,%lu,1);",dwAvatarGUID,quest_id);
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );												CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );									CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
}

/**
 *	Updates the database value of the avatar's location.
 */
void cAvatar::UpdateAvatarLocation( )
{
	//k109:  Update avatar's location.
	char szCommand[500];
	RETCODE	retcode;
	
	DWORD dwLandblock	= m_Location.m_dwLandBlock;
	DWORD flX;
	DWORD flY;
	DWORD flZ;
	DWORD flA;
	DWORD flB;
	DWORD flC;
	DWORD flW;
	DWORD dwGUID		= m_dwGUID;

//	cMasterServer::ServerMessage(ColorYellow,NULL,"Landblock: %08x %4.2f",who->m_pcAvatar->m_Location.m_dwLandBlock,myfl);

	// floating point to 32-bit hexadecimal
	flX = cDatabase::Float2Hex(m_Location.m_flX);
	flY = cDatabase::Float2Hex(m_Location.m_flY);
	flZ = cDatabase::Float2Hex(m_Location.m_flZ);
	flA = cDatabase::Float2Hex(m_Location.m_flA);
	flB = cDatabase::Float2Hex(m_Location.m_flB);
	flC = cDatabase::Float2Hex(m_Location.m_flC);
	flW = cDatabase::Float2Hex(m_Location.m_flW);

//	cMasterServer::ServerMessage(ColorYellow,NULL,"Landblock: %08x %4.2f",who->m_pcAvatar->m_Location.m_dwLandBlock,myfl);

	sprintf( szCommand, "UPDATE avatar_location SET Landblock = ('%08x'), Position_X = ('%08x'),Position_Y = ('%08x'),Position_Z = ('%08x'),Orientation_W = ('%08x'),Orientation_X = ('%08x'),Orientation_Y = ('%08x'),Orientation_Z = ('%08x') WHERE AvatarGUID = (%d);",dwLandblock, flX, flY, flZ, flA, flB, flC, flW, dwGUID );
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
	retcode = SQLExecute( cDatabase::m_hStmt );										CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
//	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

/**
 *	Updates the database value of the avatar's unassigned experience.
 *
 *	@param *who - A pointer to the client whose avatar's unassigned experience value should be updated.
 *	@param exp - The present unassigned experience value.
 */
void cAvatar::UpdateUnassignedXPDB(cClient *who, DWORD exp)
{
	if (m_dwUnassignedXP < 4294967295)
	{
		char szCommand[150];
		RETCODE	retcode;
		sprintf( szCommand, "UPDATE avatar set UnassignedXP = UnassignedXP + %lu where AvatarGUID = %lu;",exp,who->m_pcAvatar->GetGUID( ));
		retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLExecute( cDatabase::m_hStmt );
		retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	}
}
/**
 *	Updates the database value of the avatar's total experience.
 *
 *	@param *who - A pointer to the client whose avatar's total experience value should be updated.
 *	@param exp - The present total experience value.
 */
void cAvatar::UpdateTotalXPDB(cClient *who, DWORD exp)
{
	char szCommand[100];
	RETCODE	retcode;
	sprintf( szCommand, "UPDATE avatar set TotalXP = TotalXP + %i where AvatarGUID = %lu;",exp,who->m_pcAvatar->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}
/**
 *	Updates the database value of the avatar's level.
 *
 *	@param *who - A pointer to the client whose avatar's level value should be updated.
 *	@param exp - The present level value.
 */
void cAvatar::UpdateLevelDB(cClient *who)
{
	char szCommand[100];
	RETCODE	retcode;
	sprintf( szCommand, "UPDATE avatar set Level = %i where AvatarGUID = %lu;",who->m_pcAvatar->m_cStats.m_dwLevel,who->m_pcAvatar->GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS ); CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
}

/*
cMessage cAvatar::UpdateAvatarPalette()
	{
	cMessage cmReturn;

	char szCommand[255];
	RETCODE retcode;

	sprintf( szCommand, "SELECT * FROM avatar_palettes WHERE dwlinker=%d ORDER BY VectorCount;", this->m_wModelNum);
	BYTE bVectorCount;

	// Define Temp Variables
		char	readbuff01[5];
		char	readbuff02[3];
		char	readbuff03[3];

	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	int iCol = 3;
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_USHORT, &bVectorCount, sizeof( bVectorCount ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff01 , sizeof( readbuff01 ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	 	retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff02 , sizeof( readbuff02 ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
		retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, &readbuff03 , sizeof( readbuff03 ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS && i < this->m_bPaletteChange; ++i )
	{
		
		//Cube:  Change below, need to add a way to record Avatars p/t/m info.
		sscanf(readbuff01,"%04x",&pc.m_wNewPalette);
		sscanf(readbuff02,"%02x",&pc.m_ucOffset);
		sscanf(readbuff03,"%02x",&pc.m_ucLength);

		m_vectorPal[i].m_wNewPalette = pc.m_wNewPalette;
		m_vectorPal[i].m_ucOffset = pc.m_ucOffset;
		m_vectorPal[i].m_ucLength = pc.m_ucLength;

	cmReturn << m_vectorPal[i].m_wNewPalette << m_vectorPal[i].m_ucOffset << m_vectorPal[i].m_ucLength;

	retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
	
	}
	//Cube:  Palette Code
	cmReturn << WORD( 0x08A0 );

	return cmReturn;
}
*/
cMessage cAvatar::AllegianceInfo(DWORD F7B0seq)
{
	cMessage cmReturn;

	cmReturn	<< 0xF7B0 << GetGUID() << F7B0seq << 0x20L;	// Allegiance Info
	
	cAllegiance* aAllegiance ;
	
	int			numRecords = 0;
	int			numFollowers = 0;
	int			allegianceSize = 0;
	DWORD		allegianceLeader = 0;
	DWORD		dwRank = 0;
//	std::string	allegianceName = "";

	std::list< Member >	memberList;
	std::list< DWORD >	treeParentList;

	aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID);
	if (aAllegiance)
	{
		allegianceSize = aAllegiance->GetSize();	// allegiance size
//		allegianceName = aAllegiance->GetName();	// allegiance name
		allegianceLeader = aAllegiance->GetLeader();

		// Find the monarch's allegiance record
		Member memMonarch = aAllegiance->members.find(aAllegiance->GetLeader())->second;

		// Add the monarch's allegiance record
		memberList.push_back( memMonarch );
		treeParentList.push_back( 0x00000001);
		numRecords++;

		// Find the player's allegiance record
		Member memPlayer = aAllegiance->members.find(m_dwGUID)->second;
		numFollowers = DWORD(memPlayer.m_dwFollowers);	// avatar's follower count
		dwRank = DWORD(memPlayer.m_bRank);				// avatar's rank

		// Find the patron's allegiance record
		if (memPlayer.m_dwGUID != aAllegiance->GetLeader() && memPlayer.m_dwPatron != 0)
		{
			Member memPatron = aAllegiance->members.find(memPlayer.m_dwPatron)->second;
			if (memPatron.m_dwGUID != aAllegiance->GetLeader())	// if the patron is not also the monarch
			{
				// Add the patron's allegiance record
				memberList.push_back( memPatron );
				treeParentList.push_back ( memMonarch.m_dwGUID );	// the monarch is always the patron's "tree parent"
				numRecords++;
			}
		}

		if (aAllegiance->GetLeader() != m_dwGUID)	// if the player is not the monarch
		{
			// Add the player's allegiance record
			memberList.push_back( memPlayer );
			treeParentList.push_back ( memPlayer.m_dwPatron );		// the patron is always the (non-monarch) player's "tree parent"
			numRecords++;
		}

		for (int iVas = 0; iVas < MAX_VASSALS; ++iVas)
		{
			// Find vassals' allegiance record
			if (memPlayer.m_dwVassals[iVas] != 0)
			{
				// Add the vassal's allegiance record
				Member memVassal = aAllegiance->members.find(memPlayer.m_dwVassals[iVas])->second;
				memberList.push_back( memVassal );
				treeParentList.push_back ( memVassal.m_dwPatron );	// the patron is always the (non-monarch) player's "tree parent"
				numRecords++;
			}
		}
	}

	time_t curtime;
	curtime = time (NULL);	// the current time

	cmReturn	<< DWORD(dwRank)				// DWORD	personal rank (rank + any rank bonus)
				<< DWORD(allegianceSize)		// DWORD	allegiance size (monarch + followers)
				<< DWORD(numFollowers)			// DWORD	personal number of followers
				<< WORD(numRecords)				// WORD		recordCount
				<< WORD(0x30)					// WORD		unknown1
				<< 0x01000000					// DWORD	unknown2
				<< 0x0L							// DWORD	unknown3
				<< 0x0L							// DWORD	unknown4
				<< 0x0L							// DWORD	unknown5
				<< 0x0L							// DWORD	unknown6
				<< 0x0L							// DWORD	unknown7 (0-2?)
				<< 0x0L;						// DWORD	unknown8 (0-2?)
				//<< 0x0L						// DWORD	unknown9	
				//<< 0x0000XXXX					// DWORD	allegiance chat channel number
				//<< 0x0L						// DWORD	unknown10
				//<< 0x0L						// DWORD	unknown11
				//<< 0x0L						// DWORD	unknown12
				//<< 0x0L						// DWORD	unknown13
				//<< float(1)					// float	unknown14
				//<< 0x0L						// DWORD	unknown15
				//<< 0x0L						// DWORD	unknown16a
				//<< 0x0L						// DWORD	unknown16b
				//<< allegianceName.c_str()		// STRING	allegiance name
				//<< DWORD(curtime)				// DWORD	time of update

	std::list<Member>::iterator memberIter;
	std::list<DWORD>::iterator treeParentIter;
	for ( memberIter = memberList.begin(), treeParentIter = treeParentList.begin(); memberIter != memberList.end(), treeParentIter != treeParentList.end(); ++memberIter, ++treeParentIter )
	{
		// determine the character's age
		UINT64 age = curtime - (*memberIter).m_timeBirth;
		
		// determine the character's online/offline status
		DWORD online = 0x00L;
		if (cClient::FindClient( (*memberIter).m_dwGUID ))
			online = 0x01L;		// online

		DWORD type = 0x0CL;
		if ((*memberIter).m_dwGUID == allegianceLeader)
			type = type | 0x10L;	// leaders appear to always have value 0x0000001X
		//else if (unknown circumstance)
		//	type = type | 0x01L;	// unknown reason causes 0x000000XD
		
		cmReturn	<< (*treeParentIter)				// DWORD	treeParent (monarch's = 1, pre-ToD?)
					
					<< (*memberIter).m_dwGUID			// DWORD	player GUID
					<< type								// DWORD	type
					<< (*memberIter).m_passupXP			// DWORD	total allegiance experience contribution
					<< online							// DWORD	online/offline (0 = offline; 1 = online)
					<< (*memberIter).m_bGender			// BYTE		gender (1 = male; 2 = female)
					<< (*memberIter).m_bRace			// BYTE		race (1 = Aluvian; 2 = Gharu'ndim; 3 = Sho)
					<< (*memberIter).m_bRank			// BYTE		rank (1-10)	
					<< BYTE(0x00)						// align to DWORD
					<< (*memberIter).m_wLoyalty			// WORD		loyalty
					<< (*memberIter).m_wLeadership		// WORD		leadership
					<< (*memberIter).m_timeRLSwear		// DWORD	unknown
					<< DWORD(age)						// DWORD	character age (in seconds)
					<< (*memberIter).m_szName.c_str();	// STRING	character name
	}
	
	memberList.clear();
	
	return cmReturn;
}

DWORD cAvatar::GetAllegianceFromDB()
{
	DWORD allegianceID = 0;

	char szCommand[100];
	RETCODE	retcode;

	sprintf( szCommand, "SELECT AllegianceID FROM allegiance_members WHERE MemberGUID = %lu;",GetGUID( ));
	retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );					CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLExecute( cDatabase::m_hStmt );
	retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &allegianceID, sizeof( DWORD ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
	retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

	return allegianceID;
}

cMessage cAvatar::SetPackContents(DWORD F7B0seq)
{
	cMessage cmReturn;

	cmReturn << 0xF7B0L << GetGUID( ) << F7B0seq << 0x000001CA << 0x0L;

	return cmReturn;
}

void cAvatar::BreakAllegiance()
{
	cAllegiance* aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID);
	if (aAllegiance)
		aAllegiance->UpdateAvatarRecordDB(this->m_dwGUID);
}

void cAvatar::UpdateAllegianceDB()
{
	cAllegiance* aAllegiance = cAllegiance::GetAllegianceByID(m_dwAllegianceID);
	if (aAllegiance)
		aAllegiance->UpdateAvatarRecordDB(this->m_dwGUID);
}

/**
 *	Updates the database record of the avatar.
 *
 *	@param *who - A pointer to the client whose avatar's database record should be updated.
 */
void cAvatar::SaveToDB()
{
	UpdateAvatarLocation();

	if(m_dwAllegianceID != 0)
		UpdateAllegianceDB();
}