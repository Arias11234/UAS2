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
 *	@file TreasureGen.h
 */

#ifndef __TREASUREGEN_H
#define __TREASUREGEN_H

#include <algorithm>

#include "Object.h"
#include "cItemModels.h"
#include "Avatar.h"
#include "Client.h"

class TreasureGen
{
public:
	DWORD mob_lvl;
	int t_lvl;
	float workmanship;
	int value;
	int burden;
	
void GenerateCorpseTreasure(cCorpse *Corpse, cMonster *Mob, cClient *pcClient);
void GenerateChestTreasure(cChest *Chest, cClient *pcClient);

	/**	@name Player Inventory Item Creation Functions
	 *	Functions to create items in avatars' inventory.
	 *
	 *	@param *who - A pointer to the client object.
	 *	@param ItemModelID - The item model ID of the item to create.
	 *
	 *	Author: Cubem0j0
	 *	@{
	 */
void CreateAmmo				(cClient *who, DWORD ItemModelID);
void CreateArmor			(cClient *who, DWORD ItemModelID);
void CreateBook				(cClient *who, DWORD ItemModelID, DWORD GUID);
void CreateClothes			(cClient *who, DWORD ItemModelID);
void CreateFoci				(cClient *who, DWORD ItemModelID);
void CreateFood				(cClient *who, DWORD ItemModelID, DWORD GUID);
void CreateGem				(cClient *who, DWORD ItemModelID, DWORD GUID);
void CreateHealingKit		(cClient *who, DWORD ItemModelID);
void CreateJewelry			(cClient *who, DWORD ItemModelID);
void CreateLockpicks		(cClient *who, DWORD ItemModelID);
void CreateManastone		(cClient *who, DWORD ItemModelID);
void CreateMisc				(cClient *who, DWORD ItemModelID);
void CreatePack				(cClient *who, DWORD ItemModelID);
void CreatePlants			(cClient *who, DWORD ItemModelID);
//k109: Adding amount to pyreals
void CreatePyreals			(cClient *who, DWORD ItemModelID, DWORD Value, WORD Stack);
void CreateSalvage			(cClient *who, DWORD ItemModelID);
void CreateScrolls			(cClient *who, DWORD ItemModelID);
void CreateShield			(cClient *who, DWORD ItemModelID);
void CreateSpellComponents	(cClient *who, DWORD ItemModelID, DWORD GUID);
void CreateTradeNotes		(cClient *who, DWORD ItemModelID);
void CreateTradeSkillMats	(cClient *who, DWORD ItemModelID);
void CreateWand				(cClient *who, DWORD ItemModelID, DWORD GUID);
void CreateWeapon			(cClient *who, DWORD ItemModelID, DWORD GUID, DWORD fEquipped);
	//@}

	/**	@name Landblock Item Creation Functions
	 *	Functions to create items at landblock locations.
	 *
	 *	@param Loc - The location to spawn the object (i.e. m_pcAvatar->m_Location OR m_NPC->m_Location).
	 *	@param ItemModelID - The item model ID of the item to create.
	 *
	 *	Author: Cubem0j0
	 *	@{
	 */
void CreateAmmo				(cLocation Loc, DWORD ItemModelID);
void CreateArmor			(cLocation Loc, DWORD ItemModelID);
void CreateBook				(cLocation Loc, DWORD ItemModelID);
void CreateClothes			(cLocation Loc, DWORD ItemModelID);
void CreateFoci				(cLocation Loc, DWORD ItemModelID);
void CreateFood				(cLocation Loc, DWORD ItemModelID);
void CreateGem				(cLocation Loc, DWORD ItemModelID);
void CreateHealingKit		(cLocation Loc, DWORD ItemModelID);
void CreateJewelry			(cLocation Loc, DWORD ItemModelID);
void CreateLockpicks		(cLocation Loc, DWORD ItemModelID);
void CreateManastone		(cLocation Loc, DWORD ItemModelID);
void CreateMisc				(cLocation Loc, DWORD ItemModelID);
void CreatePack				(cLocation Loc, DWORD ItemModelID);
void CreatePlants			(cLocation Loc, DWORD ItemModelID);
//k109: Adding amount to pyreals
void CreatePyreals			(cLocation Loc, DWORD ItemModelID, DWORD Value, WORD Stack);
void CreateSalvage			(cLocation Loc, DWORD ItemModelID);
void CreateScrolls			(cLocation Loc, DWORD ItemModelID);
void CreateShield			(cLocation Loc, DWORD ItemModelID);
void CreateSpellComponents	(cLocation Loc, DWORD ItemModelID);
void CreateTradeNotes		(cLocation Loc, DWORD ItemModelID);
void CreateTradeSkillMats	(cLocation Loc, DWORD ItemModelID);
void CreateWand				(cLocation Loc, DWORD ItemModelID);
void CreateWeapon			(cLocation Loc, DWORD ItemModelID);
	//@}

	/**	@name NPC Inventory Item Creation Functions
	 *	Functions to create items in NPCs' inventory.
	 *
	 *	@param *pcObj - A pointer to the NPC object.
	 *	@param ItemModelID - The item model ID of the item to create.
	 *
	 *	Author: Cubem0j0
	 *	@{
	 */
void CreateAmmo				(cNPC *pcObj, DWORD ItemModelID);
void CreateArmor			(cNPC *pcObj, DWORD ItemModelID);
void CreateBook				(cNPC *pcObj, DWORD ItemModelID);
void CreateClothes			(cNPC *pcObj, DWORD ItemModelID);
void CreateFoci				(cNPC *pcObj, DWORD ItemModelID);
void CreateFood				(cNPC *pcObj, DWORD ItemModelID);
void CreateGem				(cNPC *pcObj, DWORD ItemModelID);
void CreateHealingKit		(cNPC *pcObj, DWORD ItemModelID);
void CreateJewelry			(cNPC *pcObj, DWORD ItemModelID);
void CreateLockpicks		(cNPC *pcObj, DWORD ItemModelID);
void CreateManastone		(cNPC *pcObj, DWORD ItemModelID);
void CreateMisc				(cNPC *pcObj, DWORD ItemModelID);
void CreatePack				(cNPC *pcObj, DWORD ItemModelID);
void CreatePlants			(cNPC *pcObj, DWORD ItemModelID);
void CreatePyreals			(cNPC *pcObj, DWORD ItemModelID, DWORD Value, WORD Stack);
void CreateSalvage			(cNPC *pcObj, DWORD ItemModelID);
void CreateScrolls			(cNPC *pcObj, DWORD ItemModelID);
void CreateShield			(cNPC *pcObj, DWORD ItemModelID);
void CreateSpellComponents	(cNPC *pcObj, DWORD ItemModelID);
void CreateTradeNotes		(cNPC *pcObj, DWORD ItemModelID);
void CreateTradeSkillMats	(cNPC *pcObj, DWORD ItemModelID);
void CreateWand				(cNPC *pcObj, DWORD ItemModelID);
void CreateWeapon			(cNPC *pcObj, DWORD ItemModelID);
	//@}

	/**	@name Corpse Inventory Item Creation Functions
	 *	Functions to create items in corpses' inventory.
	 *
	 *	@param *pcObj - A pointer to the corpse object.
	 *	@param ItemModelID - The item model ID of the item to create.
	 *
	 *	Author: Cubem0j0
	 *	@{
	 */
void CreateAmmo				(cCorpse *pcObj, DWORD ItemModelID);
void CreateArmor			(cCorpse *pcObj, DWORD ItemModelID);
void CreateBook				(cCorpse *pcObj, DWORD ItemModelID);
void CreateClothes			(cCorpse *pcObj, DWORD ItemModelID);
void CreateFoci				(cCorpse *pcObj, DWORD ItemModelID);
void CreateFood				(cCorpse *pcObj, DWORD ItemModelID);
void CreateGem				(cCorpse *pcObj, DWORD ItemModelID);
void CreateHealingKit		(cCorpse *pcObj, DWORD ItemModelID);
void CreateJewelry			(cCorpse *pcObj, DWORD ItemModelID);
void CreateLockpicks		(cCorpse *pcObj, DWORD ItemModelID);
void CreateManastone		(cCorpse *pcObj, DWORD ItemModelID);
void CreateMisc				(cCorpse *pcObj, DWORD ItemModelID);
void CreatePack				(cCorpse *pcObj, DWORD ItemModelID);
void CreatePlants			(cCorpse *pcObj, DWORD ItemModelID);
void CreatePyreals			(cCorpse *pcObj, DWORD ItemModelID, DWORD Value, WORD Stack);
void CreateSalvage			(cCorpse *pcObj, DWORD ItemModelID);
void CreateScrolls			(cCorpse *pcObj, DWORD ItemModelID);
void CreateShield			(cCorpse *pcObj, DWORD ItemModelID);
void CreateSpellComponents	(cCorpse *pcObj, DWORD ItemModelID);
void CreateTradeNotes		(cCorpse *pcObj, DWORD ItemModelID);
void CreateTradeSkillMats	(cCorpse *pcObj, DWORD ItemModelID);
void CreateWand				(cCorpse *pcObj, DWORD ItemModelID);
void CreateWeapon			(cCorpse *pcObj, DWORD ItemModelID);
	//@}
};

#endif