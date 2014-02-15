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
 *	@file Account.h
 *	Processes the create and delete character messages.
 */

#ifndef __ACCOUNT_H
#define __ACCOUNT_H

#include <winsock2.h>

#define CHARDATA_RACE_ALUVIAN		0x0
#define CHARDATA_RACE_GHARU			0x1
#define CHARDATA_RACE_SHO			0x2

#define CHARDATA_HAIRSTYLE_BUSINESS	0x0
#define CHARDATA_HAIRSTYLE_PUNK		0x1
#define CHARDATA_HAIRSTYLE_PONYTAIL 0x2
#define CHARDATA_HAIRSTYLE_BALD		0x3

#define CHARDATA_SEX_FEMALE			0x0
#define CHARDATA_SEX_MALE			0x1

#define CHARDATA_STARTINGTOWN_HOLTBURG_SOUTH	0x0
#define CHARDATA_STARTINGTOWN_HOLTBURG_WEST		0x1
#define CHARDATA_STARTINGTOWN_SHOUSHI_SOUTHEAST	0x2
#define CHARDATA_STARTINGTOWN_SHOUSHI_WEST		0x3
#define CHARDATA_STARTINGTOWN_YARAQ_NORTH		0x4
#define CHARDATA_STARTINGTOWN_YARAQ_EAST		0x5

#define CHARDATA_CLASS_CUSTOM		0x0
#define CHARDATA_CLASS_BOW_HUNTER	0x1
#define CHARDATA_CLASS_SWASHBUCKLER	0x2
#define CHARDATA_CLASS_LIFE_CASTER	0x3
#define CHARDATA_CLASS_WAR_MAGE		0x4
#define CHARDATA_CLASS_WAYFARER		0x5
#define CHARDATA_CLASS_SOLDIER		0x6

#define SKILL_AXE					0x01
#define SKILL_BOW					0x02
#define SKILL_CROSSBOW				0x03
#define SKILL_DAGGER				0x04
#define SKILL_MACE					0x05
#define SKILL_MELEE_DEFENSE			0x06
#define SKILL_MISSLE_DEFENSE		0x07
#define SKILL_SPEAR					0x09
#define SKILL_STAFF					0x0A
#define SKILL_SWORD					0x0B
#define SKILL_THROWN_WEAPONS		0x0C
#define SKILL_UNARMED_COMBAT		0x0D
#define SKILL_ARCANE_LORE			0x0E
#define SKILL_MAGIC_DEFENSE			0x0F
#define SKILL_MANA_CONVERSION		0x10
#define SKILL_ITEM_TINKERING		0x12
#define SKILL_ASSESS_PERSON			0x13
#define SKILL_DECEPTION				0x14
#define SKILL_HEALING				0x15
#define SKILL_JUMP					0x16
#define SKILL_LOCKPICK				0x17
#define SKILL_RUN					0x18
#define SKILL_ASSESS_CREATURE		0x1B
#define SKILL_WEAPON_TINKERING		0x1C
#define SKILL_ARMOR_TINKERING		0x1D
#define SKILL_MAGIC_ITEM_TINKERING	0x1E
#define SKILL_CREATURE_ENCHANTMENT	0x1F
#define SKILL_ITEM_ENCHANTMENT		0x20
#define SKILL_LIFE_MAGIC			0x21
#define SKILL_WAR_MAGIC				0x22
#define SKILL_LEADERSHIP			0x23
#define SKILL_LOYALTY				0x24
#define SKILL_FLETCHING				0x25
#define SKILL_ALCHEMY				0x26
#define SKILL_COOKING				0x27
#define SKILL_SALVAGE				0x28

/**
 *	The CreateCharacterMessage struct parallels the structure of the Create Character message.
 *
 *	Values from this struct are used by cDatabase to create characters. @see cDatabase::CreateAvatar
 */
#pragma pack( push, 1 )
struct CreateCharacterMessage
{
	DWORD	dwF656; 
//	DWORD	dwBeefbeef;				//0xBEEFBEEF always; probably used in place of GUID
	WORD	wLengthOfAccountName;	//Length of the account name
	char	szAccountName[40];		//40 byte account name; zero-filled
//	DWORD	dwUnk1;					
	DWORD	dwValOne;				//Constant 1
	DWORD	dwRace;					//Use CHARDATA_RACE_
	DWORD	dwSex;					//Use CHARDATA_SEX_
	DWORD	dwForeheadTexture;		//Value from a race-wide index of texures
	DWORD	dwNoseTexture;			//Value from a race-wide index of texures
	DWORD	dwChinTexture;			//Value from a race-wide index of texures
	DWORD	dwHairColor;			//Value from a race-wide index of colors
	DWORD	dwEyeColor;				//Value from a race-wide index of colors
	DWORD	dwHairStyle;			//Use CHARDATA_HAIRSTYLE_
	DWORD	dwHatType;				//Value from a race-wide index of hat types; 0xFFFFFFFF (-1) means no hat
	DWORD	dwHatColor;				//Value from a race-wide index of colors
	DWORD	dwShirtType;			//Value from a race-wide index of shirt types
	DWORD	dwShirtColor;			//Value from a race-wide index of colors
	DWORD	dwPantsType;			//Value from a race-wide index of pants
	DWORD	dwPantsColor;			//Value from a race-wide index of colors
	DWORD	dwShoeType;				//Value from a race-wide index of shoe types
	DWORD	dwShoeColor;			//Value from a race-wide index of colors
	
	//Values range from 0x0000000000000000 to 3FF0000000000000000 (0-1)
	//Indicate degree of color shading
	double	dblSkinShade;			
	double	dblHairShade;
	double	dblHatShade;
	double	dblShirtShade;
	double	dblPantsShade;
	double	dblShoeShade;

	DWORD	dwProfession;			//CHARDATA_PROFESSION_

	DWORD	dwStrength;
	DWORD	dwEndurance;
	DWORD	dwCoordination;
	DWORD	dwQuickness;		
	DWORD	dwFocus;
	DWORD	dwSelf;

	DWORD	dwUnk2;
	DWORD	dwUnk3;
	
	DWORD	dwNumSkills;			//Portal.dat file dependent
	DWORD	dwSkillStatus[0x30];	
	// 0 - Skill ID not used
	// 1 - Skill Unusable / Untrained
	// 2 - Skill Trained
	// 3 - Skill Specialized
	
	WORD	wNameLength;
	char	szName[40];
	DWORD	dwUnk4;					//Always 0xFFFFFFFF?
	DWORD	dwUnk5;					//Random value that has ranged from 0x02-0x0C
	DWORD	dwStartingPlace;		//CHARDATA_STARTINGTOWN_
	DWORD	dwUnk6;					//Random value that has ranged from 0x53-0x157
};
#pragma pack( pop ) 

/**
 *	The AvatarDeleteMessage struct parallels the structure of the Delete Character message.
 */
#pragma pack( push, 1 )
struct AvatarDeleteMessage 
{
	DWORD dwF655;
//	DWORD dwBeef;
	WORD wNameLength;
	char szAccountName[40];
//	DWORD dw74;
	DWORD dwSlot;
};
#pragma pack( pop ) 

#endif	// #ifndef __ACCOUNT_H