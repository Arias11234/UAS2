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
 *	@file TreasureGen.cpp
 *	Encapsulates treasure generation functionality.
 */

#include "Object.h"
#include "cItemModels.h"
#include "Avatar.h"
#include "Client.h"
#include "WorldManager.h"
#include "TreasureGen.h"

void TreasureGen::GenerateCorpseTreasure(cCorpse *Corpse, cMonster *Mob, cClient *pcClient)
{
	//First, get the mobs level
	mob_lvl = Mob->m_dwLevel;

	//Base the treasure level off the mob level
	if(mob_lvl <= 5)
	{
		t_lvl = 1;
	}
	if(mob_lvl > 5 && mob_lvl <= 20)
	{
		t_lvl = 2;
	}
	if(mob_lvl > 20 && mob_lvl <= 30)
	{
		t_lvl = 3;
	}
	if(mob_lvl > 30 && mob_lvl <= 40)
	{
		t_lvl = 4;
	}
	if(mob_lvl > 40 && mob_lvl <= 60)
	{
		t_lvl = 5;
	}
	if(mob_lvl > 60 && mob_lvl <= 80)
	{
		t_lvl = 6;
	}
	if(mob_lvl > 80)
	{
		t_lvl = 7;
	}
	if(mob_lvl > 100)
	{
		t_lvl = 8;
	}

	//Roll a random number to see how many items exist on corpse
	int iAmt = rand() % 4 + 1;

	//Now select the types of items that will be created:
	int iType = rand() % 10 + 1;
}

void TreasureGen::GenerateChestTreasure(cChest *Chest, cClient *pcClient)
{

}

/*	GENERATE ITEM IN PLAYER INVENTORY	*/
void TreasureGen::CreateAmmo(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cAmmo* aAmmo = new cAmmo(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_wStack, pcModel->m_wStackLimit);
	who->AddPacket( WORLD_SERVER, aAmmo->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aAmmo);
}

void TreasureGen::CreateArmor(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing, pcModel->m_fProt_Piercing, pcModel->m_fProt_Bludgeon, pcModel->m_fProt_Fire, pcModel->m_fProt_Cold, pcModel->m_fProt_Acid, pcModel->m_fProt_Electric);
	who->AddPacket( WORLD_SERVER, aArmor->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aArmor);
}

void TreasureGen::CreateBook(cClient *who, DWORD ItemModelID, DWORD GUID)
{
	if (GUID == NULL || GUID == 0)
		GUID = cWorldManager::NewGUID_Object();

	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cBooks* aBook = new cBooks(GUID,who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aBook->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aBook);
}

void TreasureGen::CreateClothes(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cClothes* aShirt = new cClothes(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aShirt->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aShirt);
}

void TreasureGen::CreateGem(cClient *who, DWORD ItemModelID, DWORD GUID)
{
	if (GUID == NULL || GUID == 0)
		GUID = cWorldManager::NewGUID_Object();

	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cGems* aGem = new cGems(GUID,who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aGem->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aGem);
}

void TreasureGen::CreateFoci(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFoci* aFoci = new cFoci(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aFoci->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aFoci);
}

void TreasureGen::CreateFood(cClient *who, DWORD ItemModelID, DWORD GUID)
{
	if (GUID == NULL || GUID == 0)
		GUID = cWorldManager::NewGUID_Object();

	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFood* aMeal = new cFood(GUID,who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wStack, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
	who->AddPacket( WORLD_SERVER, aMeal->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aMeal);
}

void TreasureGen::CreateHealingKit(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cHealingCon* aKit = new cHealingCon(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_wUses, pcModel->m_wUseLimit);
	who->AddPacket( WORLD_SERVER, aKit->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aKit);
}

void TreasureGen::CreateJewelry(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cJewelry* aRing = new cJewelry(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aRing->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aRing);
}

void TreasureGen::CreateLockpicks(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cLockpicks* aPicks = new cLockpicks(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_wUses, pcModel->m_wUseLimit);
	who->AddPacket( WORLD_SERVER, aPicks->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aPicks);
}

void TreasureGen::CreateManastone(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cManaStones* aManastone = new cManaStones(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aManastone->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aManastone);
}

void TreasureGen::CreateMisc(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cMisc* aMisc = new cMisc(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	who->AddPacket( WORLD_SERVER, aMisc->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aMisc);
}

void TreasureGen::CreatePack(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPack* aPack = new cPack(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aPack->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aPack);
}

void TreasureGen::CreatePlants(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPlants* aPlant = new cPlants(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aPlant->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aPlant);
}

void TreasureGen::CreatePyreals(cClient *who, DWORD ItemModelID, DWORD Value, WORD Stack)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPyreals* aPyreal = new cPyreals(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription, Value, Stack, pcModel->m_wStackLimit);
	who->AddPacket( WORLD_SERVER, aPyreal->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aPyreal);
}

void TreasureGen::CreateSalvage(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSalvage* aSalvage = new cSalvage(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aSalvage->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aSalvage);
}

void TreasureGen::CreateScrolls(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cScrolls* aScroll = new cScrolls(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	who->AddPacket( WORLD_SERVER, aScroll->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aScroll);
}

void TreasureGen::CreateShield(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cShield* aShield = new cShield(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_dwArmor_Level);
	who->AddPacket( WORLD_SERVER, aShield->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aShield);
}

void TreasureGen::CreateSpellComponents(cClient *who, DWORD ItemModelID, DWORD GUID)
{
	if (GUID == NULL || GUID == 0)
		GUID = cWorldManager::NewGUID_Object();

	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSpellComps* aRegs = new cSpellComps(GUID,who->m_pcAvatar->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,1, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aRegs->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aRegs);
}

void TreasureGen::CreateTradeNotes(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeNotes* aNote = new cTradeNotes(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aNote->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aNote);
}

void TreasureGen::CreateTradeSkillMats(cClient *who, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeSkillMats* aMats = new cTradeSkillMats(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aMats->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aMats);
}

void TreasureGen::CreateWand(cClient *who, DWORD ItemModelID, DWORD GUID)
{
	if (GUID == NULL || GUID == 0)
		GUID = cWorldManager::NewGUID_Object();

	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWands* aWand = new cWands(GUID,who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	who->AddPacket( WORLD_SERVER, aWand->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aWand);
}

void TreasureGen::CreateWeapon(cClient *who, DWORD ItemModelID, DWORD GUID, DWORD fEquipped)
{
	if (GUID == NULL || GUID == 0)
		GUID = cWorldManager::NewGUID_Object();

	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),who->m_pcAvatar->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
	aWeapon->m_fEquipped = fEquipped;
	who->AddPacket( WORLD_SERVER, aWeapon->CreatePacketContainer(who->m_pcAvatar->GetGUID(),pcModel->m_dwModelID),3);
	who->m_pcAvatar->AddInventory(aWeapon);

	/*
	cMessage cmInsInv;
	cmInsInv << 0xF7B0L << who->m_pcAvatar->GetGUID( ) << who->m_pcAvatar->m_dwF7B0Sequence++ << 0x0022L << aWeapon->GetGUID() << who->m_pcAvatar->GetGUID( ) << 0L << 0L;
	who->AddPacket( WORLD_SERVER, cmInsInv, 4 );
	*/
}

/*	GENERATE ITEM AT LANDBLOCK LOCATION	*/
void TreasureGen::CreateAmmo(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cAmmo* aAmmo = new cAmmo(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aAmmo,true);
}

void TreasureGen::CreateArmor(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aArmor,true);
}

void TreasureGen::CreateBook(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cBooks* aBook = new cBooks(cWorldManager::NewGUID_Object(),Loc,ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aBook,true);
}

void TreasureGen::CreateClothes(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cClothes* aShirt = new cClothes(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aShirt,true);
}

void TreasureGen::CreateGem(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cGems* aGem = new cGems(cWorldManager::NewGUID_Object(),Loc,ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aGem,true);
}

void TreasureGen::CreateFoci(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFoci* aFoci = new cFoci(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aFoci,true);
}

void TreasureGen::CreateFood(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFood* aMeal = new cFood(cWorldManager::NewGUID_Object(),Loc,ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription, pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
	cWorldManager::AddObject(aMeal,true);
}

void TreasureGen::CreateHealingKit(cLocation Loc, DWORD ItemModelID)
{

}

void TreasureGen::CreateJewelry(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cJewelry* aRing = new cJewelry(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aRing,true);
}

void TreasureGen::CreateLockpicks(cLocation Loc, DWORD ItemModelID)
{

}

void TreasureGen::CreateManastone(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cManaStones* aManastone = new cManaStones(cWorldManager::NewGUID_Object(),Loc,ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aManastone,true);
}

void TreasureGen::CreateMisc(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cMisc* aMisc = new cMisc(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aMisc,true);
}

void TreasureGen::CreatePack(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPack* aPack = new cPack(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aPack,true);
}

void TreasureGen::CreatePlants(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPlants* aPlant = new cPlants(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aPlant,true);
}

void TreasureGen::CreatePyreals(cLocation Loc, DWORD ItemModelID, DWORD Value, WORD Stack)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPyreals* aPyreal = new cPyreals(cWorldManager::NewGUID_Object(),Loc,ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription, Value, Stack, pcModel->m_wStackLimit);
	cWorldManager::AddObject(aPyreal,true);
}

void TreasureGen::CreateSalvage(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSalvage* aSalvage = new cSalvage(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aSalvage,true);
}

void TreasureGen::CreateScrolls(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cScrolls* aScroll = new cScrolls(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aScroll,true);
}

void TreasureGen::CreateShield(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cShield* aShield = new cShield(cWorldManager::NewGUID_Object(),Loc,ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription, pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_dwArmor_Level);
	cWorldManager::AddObject(aShield,true);
}

void TreasureGen::CreateSpellComponents(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSpellComps* aRegs = new cSpellComps(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aRegs,true);
}

void TreasureGen::CreateTradeNotes(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeNotes* aNote = new cTradeNotes(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	cWorldManager::AddObject(aNote,true);
}

void TreasureGen::CreateTradeSkillMats(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeSkillMats* aMats = new cTradeSkillMats(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aMats,true);
}

void TreasureGen::CreateWand(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWands* aWand = new cWands(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	cWorldManager::AddObject(aWand,true);
}

void TreasureGen::CreateWeapon(cLocation Loc, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),Loc,ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
	cWorldManager::AddObject(aWeapon,true);
}

/*	GENERATE ITEM IN NPC INVENTORY	*/
void TreasureGen::CreateAmmo(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cAmmo* aAmmo = new cAmmo(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_wStack, pcModel->m_wStackLimit);
	pcObj->AddInventory(aAmmo);
}

void TreasureGen::CreateArmor(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing, pcModel->m_fProt_Piercing, pcModel->m_fProt_Bludgeon, pcModel->m_fProt_Fire, pcModel->m_fProt_Cold, pcModel->m_fProt_Acid, pcModel->m_fProt_Electric);
	pcObj->AddInventory(aArmor);
}

void TreasureGen::CreateBook(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cBooks* aBook = new cBooks(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aBook);
}

void TreasureGen::CreateClothes(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cClothes* aShirt = new cClothes(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aShirt);
}

void TreasureGen::CreateGem(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cGems* aGem = new cGems(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aGem);
}

void TreasureGen::CreateFoci(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFoci* aFoci = new cFoci(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aFoci);
}

void TreasureGen::CreateFood(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFood* aMeal = new cFood(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wStack, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
	pcObj->AddInventory(aMeal);
}

void TreasureGen::CreateHealingKit(cNPC *pcObj, DWORD ItemModelID)
{

}

void TreasureGen::CreateJewelry(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cJewelry* aRing = new cJewelry(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aRing);
}

void TreasureGen::CreateLockpicks(cNPC *pcObj, DWORD ItemModelID)
{

}

void TreasureGen::CreateManastone(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cManaStones* aManastone = new cManaStones(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aManastone);
}

void TreasureGen::CreateMisc(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cMisc* aMisc = new cMisc(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	pcObj->AddInventory(aMisc);
}

void TreasureGen::CreatePack(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPack* aPack = new cPack(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aPack);
}

void TreasureGen::CreatePlants(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPlants* aPlant = new cPlants(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aPlant);
}

void TreasureGen::CreatePyreals(cNPC *pcObj, DWORD ItemModelID, DWORD Value, WORD Stack)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPyreals* aPyreal = new cPyreals(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription, Value, Stack, pcModel->m_wStack);
	pcObj->AddInventory(aPyreal);
}

void TreasureGen::CreateSalvage(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSalvage* aSalvage = new cSalvage(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aSalvage);
}

void TreasureGen::CreateScrolls(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cScrolls* aScroll = new cScrolls(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	pcObj->AddInventory(aScroll);
}

void TreasureGen::CreateShield(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cShield* aShield = new cShield(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_dwArmor_Level);
	pcObj->AddInventory(aShield);
}

void TreasureGen::CreateSpellComponents(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSpellComps* aRegs = new cSpellComps(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, 1, pcModel->m_wBurden);
	pcObj->AddInventory(aRegs);
}

void TreasureGen::CreateTradeNotes(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeNotes* aNote = new cTradeNotes(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aNote);
}

void TreasureGen::CreateTradeSkillMats(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeSkillMats* aMats = new cTradeSkillMats(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aMats);
}

void TreasureGen::CreateWand(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWands* aWand = new cWands(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aWand);
}

void TreasureGen::CreateWeapon(cNPC *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
	pcObj->AddInventory(aWeapon);
}

/*	GENERATE ITEM IN A CORPSE INVENTORY	*/
void TreasureGen::CreateAmmo(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cAmmo* aAmmo = new cAmmo(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_wStack, pcModel->m_wStackLimit);
	pcObj->AddInventory(aAmmo);
}

void TreasureGen::CreateArmor(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cArmor* aArmor = new cArmor(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden,pcModel->m_dwArmor_Level,pcModel->m_fProt_Slashing, pcModel->m_fProt_Piercing, pcModel->m_fProt_Bludgeon, pcModel->m_fProt_Fire, pcModel->m_fProt_Cold, pcModel->m_fProt_Acid, pcModel->m_fProt_Electric);
	pcObj->AddInventory(aArmor);
}

void TreasureGen::CreateBook(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cBooks* aBook = new cBooks(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aBook);
}

void TreasureGen::CreateClothes(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cClothes* aShirt = new cClothes(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aShirt);
}

void TreasureGen::CreateGem(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cGems* aGem = new cGems(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aGem);
}

void TreasureGen::CreateFoci(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFoci* aFoci = new cFoci(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aFoci);
}

void TreasureGen::CreateFood(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cFood* aMeal = new cFood(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wStack, pcModel->m_wBurden, pcModel->m_dwVitalID, pcModel->m_vital_affect);
	pcObj->AddInventory(aMeal);
}

void TreasureGen::CreateHealingKit(cCorpse *pcObj, DWORD ItemModelID)
{

}

void TreasureGen::CreateJewelry(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cJewelry* aRing = new cJewelry(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aRing);
}

void TreasureGen::CreateLockpicks(cCorpse *pcObj, DWORD ItemModelID)
{

}

void TreasureGen::CreateManastone(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cManaStones* aManastones = new cManaStones(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aManastones);
}

void TreasureGen::CreateMisc(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cMisc* aMisc = new cMisc(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	pcObj->AddInventory(aMisc);
}

void TreasureGen::CreatePack(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPack* aPack = new cPack(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aPack);
}

void TreasureGen::CreatePlants(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPlants* aPlant = new cPlants(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aPlant);
}

void TreasureGen::CreatePyreals(cCorpse *pcObj, DWORD ItemModelID, DWORD Value, WORD Stack)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cPyreals* aPyreal = new cPyreals(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription, Value, Stack, pcModel->m_wStackLimit);
	pcObj->AddInventory(aPyreal);
}

void TreasureGen::CreateSalvage(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSalvage* aSalvage = new cSalvage(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aSalvage);
}

void TreasureGen::CreateScrolls(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cScrolls* aScroll = new cScrolls(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription);
	pcObj->AddInventory(aScroll);
}

void TreasureGen::CreateShield(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cShield* aShield = new cShield(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden, pcModel->m_dwArmor_Level);
	pcObj->AddInventory(aShield);
}

void TreasureGen::CreateSpellComponents(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cSpellComps* aRegs = new cSpellComps(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, 1 ,pcModel->m_wBurden);
	pcObj->AddInventory(aRegs);
}

void TreasureGen::CreateTradeNotes(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeNotes* aNote = new cTradeNotes(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aNote);
}

void TreasureGen::CreateTradeSkillMats(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cTradeSkillMats* aMats = new cTradeSkillMats(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aMats);
}

void TreasureGen::CreateWand(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWands* aWand = new cWands(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue, pcModel->m_wBurden);
	pcObj->AddInventory(aWand);
}

void TreasureGen::CreateWeapon(cCorpse *pcObj, DWORD ItemModelID)
{
	cItemModels *pcModel = cItemModels::FindModel(ItemModelID);
	cWeapon* aWeapon = new cWeapon(cWorldManager::NewGUID_Object(),pcObj->GetGUID(),ItemModelID,1.0,TRUE,pcModel->m_wIcon,pcModel->m_strName,pcModel->m_strDescription,pcModel->m_dwValue,pcModel->m_wBurden,pcModel->m_bWieldType,pcModel->m_dwIconHighlight,pcModel->m_fWorkmanship,pcModel->m_dwMaterialType,pcModel->m_dwWeaponDamage,pcModel->m_dwWeaponSpeed,pcModel->m_dwWeaponSkill,pcModel->m_dwDamageType,pcModel->m_dWeaponVariance,pcModel->m_dWeaponModifier,pcModel->m_dWeaponPower,pcModel->m_dWeaponAttack);
	pcObj->AddInventory(aWeapon);
}