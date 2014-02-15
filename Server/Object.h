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
 *	@file Object.h
 *	Implements general functionality for all objects.
 *	All object types inherit from this class.
 */

#ifndef __OBJECT_H
#define __OBJECT_H

#include <string>
#include "math.h"
#include <algorithm>
#include "stdio.h"
#include "stdlib.h"

#include "Message.h"
#include "cModels.h"
#include "cMagicModels.h"
#include "cItemModels.h"

/*
*		**Item Types**
*		 0 - General Items (Calling stone, Wasp wings, etc)
*		 1 - Weapons*
*		 2 - Food*
* 		 3 - Armor*
*		 4 - Books*
*		 5 - Scrolls*
*		 6 - Healing Kits*
*		 7 - Lock Picks/Keys*
*		 8 - Wands/Casting items*
*		 9 - Pyreals*
* 		10 - Mana Stones*
*		11 - Missile Weapon/Ammo*
*		12 - Shield*
*		13 - Spell Components*
*		14 - Gems*
*		15 - Trade Notes*
*		16 - Trade Skill Items*
*		17 - Plants*
*		18 - Clothes*
*		19 - Jewelry*
* 		20 - Containers/packs*
*		21 - Salvage*
*		22 - Foci*
*/

//Cubem0j0:  Apparently added as skill IDs 
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
#define PAI							3.14159265

class cWeapon;
class cWand;
class cArmor;
class cDoor;
class cChest;
class cFood;

struct cVital {
	WORD	m_wID;			//Store in database.
	DWORD	m_dwIncreases;	//Store in database.
	DWORD	m_dwCurrent;	//Do not store in database. Can be calculated from m_dwIncreases.
	DWORD	m_dwXP;			//Do not store in database. Can be calculated from m_dwIncreases.
	long	m_lTrueCurrent;
	
	inline cVital& operator=( DWORD dw )
	{
		long lDiff		= dw - m_dwCurrent;
		m_dwCurrent		= dw;
		m_dwIncreases	+= lDiff;
		return *this;
	}
};

typedef struct cObjStats
{
	DWORD	m_dwLevel; 
	DWORD	m_dwStr; 
	DWORD	m_dwEnd; 
	DWORD	m_dwQuick; 
	DWORD	m_dwCoord; 
	DWORD	m_dwFocus;
	DWORD	m_dwSelf;
	DWORD	m_dwSpecies;

	cObjStats( )
	{
		m_dwLevel	= 0x1; 
		m_dwStr		= 0x0; 
		m_dwEnd		= 0x0; 
		m_dwQuick	= 0x0; 
		m_dwCoord	= 0x0; 
		m_dwFocus	= 0x0;
		m_dwSelf	= 0x0;
		m_dwSpecies	= 0x28;
	}
} cObjStats;

struct cAttribute {
	WORD	m_wID;			//Store in database.
	DWORD	m_dwIncrement;	//Store in database.
	DWORD	m_dwCurrent;	//Store in database.
	DWORD	m_dwXP;			//Store in database.

	inline cAttribute& operator=( DWORD dw )
	{
		m_dwCurrent = dw;
		return *this;
	}
	
	inline DWORD operator+( cAttribute& rhv )
	{
		return m_dwCurrent + rhv.m_dwCurrent;
	}
	
	inline DWORD operator/( DWORD dw )
	{
		return m_dwCurrent / dw;
	}
};

struct cSkill {
	WORD	m_wID;			// Store in database.
	WORD	m_wStatus;		// Store in database.
	DWORD	m_dwIncreases;	// Store in database.
	DWORD	m_dwTotal;		// Do not store in database.
	DWORD	m_dwBase;		// Do not store in database.
	DWORD	m_dwBonus;		// Do not store in database.
	DWORD	m_dwXP;			// Do not store in database. Can be calculated from m_dwStatus and m_dwIncreases.
	DWORD	m_dwFreeXP;		// Do not store in database. Can be calculated from m_dwStatus.

	inline cSkill& operator=( DWORD dw )
	{
		m_dwIncreases = dw;
		return *this;
	}
	inline cSkill& operator=( cAttribute& rhv )
	{
		m_dwIncreases = rhv.m_dwCurrent;
		return *this;
	}
};

struct cStats {
	DWORD		m_dwLevel;
	cAttribute	m_lpcAttributes[6];
	cVital		m_lpcVitals[3];
	cSkill		m_lpcSkills[40];
};

int calc_vec_to_deg(int vx, int vy);

class cObject
{
	friend class cDatabase;
	friend class cPortalDat;
public:
	/**
	 *	A constructor.
	 */
	cObject	( cLocation *pcLoc ) 
	{ 
		if ( pcLoc ) 
			SetLocation( pcLoc ); 
		m_bInventorySequence = -1;
		m_wPositionSequence = 0;
		m_wNumPortals = 0;
		m_wNumLogins = 0x3C;
		m_fEquipped = 0;
		m_fIsOwned = FALSE;
		m_fIsCasting = FALSE;
		m_wModelSeq = 0x0;
		m_bWearSeq = 0x0;
		m_wNumAnims = 0x0000;
		m_wCurAnim	= 0x0000;
		m_wMeleeSequence	= 0x0000;
		m_dwF7B0Sequence	= 0x00000000;
		m_bStatSequence	= 0x00;
		m_fDeadOrAlive = true;
	}
	/**
	 *	A constructor.
	 */
	cObject( ) 
	{
		m_bInventorySequence = -1;
		m_wPositionSequence = 0;
		m_wNumLogins = 0x3C;
		m_fEquipped = 0;
		m_fIsOwned = FALSE;
		m_fIsCasting = FALSE;
		m_wNumAnims = 0x0000;
		m_wCurAnim = 0x0000;
		m_wMeleeSequence	= 0x0000;
		m_dwF7B0Sequence	= 0x00000000;
		m_bStatSequence	= 0x00;
		m_fDeadOrAlive = true;
	}
	virtual ~cObject( )	{}

	void SetLocation( double dNS, double dEW );
	inline void SetLocation( cLocation& Loc )
	{
		m_Location.m_dwLandBlock	= Loc.m_dwLandBlock;
		m_Location.m_flX			= Loc.m_flX;
		m_Location.m_flY			= Loc.m_flY;
		m_Location.m_flZ			= Loc.m_flZ;
		m_Location.m_flA			= Loc.m_flA;
		m_Location.m_flB			= Loc.m_flB;
		m_Location.m_flC			= Loc.m_flC;
		m_Location.m_flW			= Loc.m_flW;
	}	
	inline void SetLocation( cLocation *pcLoc )
	{
		m_Location.m_dwLandBlock	= pcLoc->m_dwLandBlock;
		m_Location.m_flX			= pcLoc->m_flX;
		m_Location.m_flY			= pcLoc->m_flY;
		m_Location.m_flZ			= pcLoc->m_flZ;
		m_Location.m_flA			= pcLoc->m_flA;
		m_Location.m_flB			= pcLoc->m_flB;
		m_Location.m_flC			= pcLoc->m_flC;
		m_Location.m_flW			= pcLoc->m_flW;
	}	
	inline void SetLocation( DWORD dwLandBlock, float flX, float flY, float flZ, float flA, float flW )
	{
		m_Location.m_dwLandBlock	= dwLandBlock;
		m_Location.m_flX			= flX;
		m_Location.m_flY			= flY;
		m_Location.m_flZ			= flZ;
		m_Location.m_flA			= flA;
		m_Location.m_flW			= flW;
	}

	inline void SetSpawnLoc( cLocation *pcLoc )
	{
		m_SpawnLoc.m_dwLandBlock	= pcLoc->m_dwLandBlock;
		m_SpawnLoc.m_flX			= pcLoc->m_flX;
		m_SpawnLoc.m_flY			= pcLoc->m_flY;
		m_SpawnLoc.m_flZ			= pcLoc->m_flZ;
		m_SpawnLoc.m_flA			= pcLoc->m_flA;
		m_SpawnLoc.m_flB			= pcLoc->m_flB;
		m_SpawnLoc.m_flC			= pcLoc->m_flC;
		m_SpawnLoc.m_flW			= pcLoc->m_flW;
	}
	inline void SetSpawnLoc( cLocation& Loc )
	{
		m_SpawnLoc.m_dwLandBlock	= Loc.m_dwLandBlock;
		m_SpawnLoc.m_flX			= Loc.m_flX;
		m_SpawnLoc.m_flY			= Loc.m_flY;
		m_SpawnLoc.m_flZ			= Loc.m_flZ;
		m_SpawnLoc.m_flA			= Loc.m_flA;
		m_SpawnLoc.m_flB			= Loc.m_flB;
		m_SpawnLoc.m_flC			= Loc.m_flC;
		m_SpawnLoc.m_flW			= Loc.m_flW;
	}
	
	//Cube: No longer need this.
	//cItemModels *cfim;

	virtual cMessage	LocationPacket	( );
	virtual cMessage	CreatePacket	( )									= 0;
	virtual cMessage	Animation		( WORD wAnim, float flPlaySpeed );
	virtual cMessage	Animation		( WORD wAnim, float flPlaySpeed, BYTE bActivity );
	virtual void		Assess			( cClient *pcAssesser ){}
	virtual void		Action			( cClient *pcActioner )				= 0;
	virtual void		Attack			( cClient *pcAttacker, float flDamageSlider, DWORD F7B0Sequence );
	virtual void		SpellAttack		( cClient *pcAttacker, cObject *pcWarSpell, DWORD F7B0Sequence );
	virtual cMessage	AdjustBar		( DWORD dwGUID, DWORD F7B0Sequence );
	
	// Remove the object
	static  cMessage	RemoveObj		( DWORD dwGUID );
	
	//Enters/exits combat stance
	virtual cMessage	ChangeCombatMode( bool fMode );
	// Chooses a combat animation
	virtual cMessage	CombatAnimation	( DWORD dwTarget, WORD wAttackAnim );
	// Calculates objects melee damage
	virtual	DWORD		CalculateDamage	( int strength, float flPower, float flResistance );
	// Turn to Face Target
	virtual cMessage	TurnToTarget	( float flHeading, DWORD dwTargetGUID );
	virtual float		GetRange		( DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ);
	virtual float		GetHeadingTarget( DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ );
	virtual cMessage	MoveToTarget	( cClient *pcWho );
	virtual cMessage	MoveTarget		( cClient *pcWho );
	virtual cMessage	ReturnToSpawn	( );
	virtual void		ReSpawn			( cObject *pcObject );
	virtual cLocation	CoordLoc		( float dNS, float dEW );

	cLocation			EstimateLoc		( float flHeading, float flSpeed, float flDistToTarget,float flTime );
	virtual cMessage	SetPosition		( );

//	virtual void		Split			( cClient* who, DWORD item_guid, DWORD slot, DWORD value );

	virtual inline DWORD GetMonsterModelID( )
	{
		return m_dwMonsterModel;
	}
	
	virtual inline DWORD GetItemModelID()
	{
		return m_dwItemModelID;
	}

	// Returns the GUID of the object
	inline DWORD GetGUID() 
	{ 
		return m_dwGUID; 
	}
	
	inline DWORD SetGUID(cObject *pcObj, DWORD nGUID)
	{
		pcObj->m_dwGUID = nGUID;
		return pcObj->m_dwGUID;
	}

	// Returns Container ID
	inline DWORD GetContainer(DWORD dwGUID)
	{
		return m_dwContainer;
	}
	// Object name
	inline const char *Name() 
	{
		return m_strName.c_str(); 
	}

	inline const char *TokenlessName()
	{
		const char *szName = m_strName.c_str();

		if ( *szName == '+' )	return szName + 1;
		else					return szName;
	}

	
	inline void SetStatic( BOOL state )
	{
		m_fStatic = state;
	}
	
	inline BOOL IsStatic()
	{
		return m_fStatic;
	}
	
	/*
	Cubem0j0: Had to add this.  Objects need a type.  Before I added this when you used an object it
	looked for the landblock info.  If it was not there (that item was in a container) then the program 
	crashed.

	Object types:
	2 = Stamina food

	*/
	void	SetType(int type)
	{
		item_type = type;
	}
	int GetType()
	{
		return item_type;
	}

	//Cubem0j0:  Adding a way to set a state on an object..currently used in corpses
	void	SetState(int state)
	{
		m_wState = state;
	}
	int		GetState()
	{
		return m_wState;
	}

	cLocation	m_Location;
	cLocation	m_SpawnLoc;
	
	BYTE		m_bInventorySequence;
	WORD		m_wPositionSequence;
	WORD		m_wNumLogins;
	WORD		m_wNumPortals;
	int			m_fEquipped;
	BOOL		m_fIsOwned;
	BOOL		m_fIsCasting;
	DWORD		m_dwContainer;
	WORD		m_wModelSeq;
	BYTE		m_bWearSeq;
	WORD		m_wNumEquips;
	WORD		m_wState;
	WORD		m_wNumAnims;
	DWORD		m_dwDoorState;
	WORD		m_wPortalMode;
	DWORD		m_dwObjectFlags1;
	DWORD		m_dwObjectFlags2;
	cLocation   m_LSLoc;
	cLocation   m_HRLoc;
	std::string m_strName;
	std::string m_strDescription;
	WORD		m_wCurAnim;
	WORD		m_wMeleeSequence;
	DWORD		m_dwF7B0Sequence;
	DWORD		m_dwConfirmSequence;
	BYTE		m_bStatSequence;
	bool		m_fCombatMode;
	BYTE		m_bIdleAnim;
	cVelocity	m_Velocity;
	DWORD		m_dwReSpawn;
	DWORD		m_dwDecay;
	DWORD		m_dwChase;
	DWORD		m_dwInfluence;
	DWORD		m_dwExp_Value;
	bool		m_fDeadOrAlive;
	DWORD		m_dwSpellModel;
	DWORD		m_dwSpellID;
	DWORD		m_dwFlagCount;
	int			item_type;
	WORD		m_wUses;
	DWORD		m_dwItemModelID;
	float		m_fApproachDistance;
//	DWORD		m_dwHouseType;
	DWORD		m_dwHouseGUID;
	DWORD		m_dwOwnerID;
	DWORD		m_dwQuantity;

	//k109:  Item specific stuff
	//Food
	DWORD		m_dwVitalID;
	DWORD		m_dwAmount;
	//Armor
	DWORD		m_dwActivate_Req_SkillID;
	DWORD		m_dwActivate_Req_Skill_Level;
	DWORD		m_dwArmor_Level;
	float		m_fProt_Slashing;
	float		m_fProt_Piercing;
	float		m_fProt_Bludgeon;
	float		m_fProt_Fire;
	float		m_fProt_Cold;
	float		m_fProt_Acid;
	float		m_fProt_Electric;
	//Pyreals
	DWORD		m_dwValue;
	WORD		m_wStack;
	WORD		m_wStackLimit;

	/* Palettes are unique to each apparel item */
	//(Texture and model information is constant and populated to the item model)
	// Item palette vectors
	int				m_intColor;

	BYTE			m_bPaletteChange;
	DWORD			m_wPaletteVector;
	sPaletteChange	m_vectorPal[255];
	// Item clothing palette vectors
	BYTE			m_bWearPaletteChange;
	DWORD			m_wWearPaletteVector;
	sPaletteChange	m_WearVectorPal[255];

	//the object's enchantment list
	std::list< cEnchantment * >  m_lstEnchantments;

protected:
	DWORD		m_dwGUID;
	DWORD		m_dwModel;
	WORD		m_wIcon;
	BOOL		m_fSelectable;
	float		m_flScale;
	BOOL		m_fStatic;
	DWORD		m_dwMode;
	DWORD		m_dwMonsterModel;
};

class cBiotic : public cObject
{
public:
	cBiotic	( cLocation *pcLoc ) 
		:	cObject( pcLoc ) 
	{
		m_bInventorySequence = -1;
		m_wPositionSequence = 0;
		m_wNumPortals = 0;
		m_wNumLogins = 0x3C;
		m_fIsOwned = FALSE;
	}

	cBiotic( )
	{
		m_bInventorySequence = -1;
		m_wPositionSequence = 0;
		m_wNumPortals = 0;
		m_wNumLogins = 0x3C;
	}
	virtual ~cBiotic( ) {}
	virtual void Action( cClient *pcClient )	{}

protected:
	cStats m_Stats;
};

#define NPCMODE_SINGLE		1
#define NPCMODE_MULTI		2
#define NPCMODE_RANDOM		3

class cNPC : public cObject
{
	friend class cClient;

public:
	cNPC();
	/**
	 *	A constructor.
	 *
	 *	Called whenever an NPC object is initialized.
	 */
	cNPC(DWORD dwGUID, DWORD dwNPCModelID, char *szName, WORD wGender, cLocation *pcLoc = NULL, DWORD dwSellCategories = 0x00000000 );

	void		Assess				(cClient *pcAssesser );
	cMessage	CreatePacket		( );
	void		Action				(cClient* who);
	//Cube: Give item, for quests, etc.
	void		GiveItem			(cClient *who, DWORD itemid, DWORD npcid);
	//Cube: Buy Item from Vendor
	void		BuyItem				(cClient *who, DWORD VendorID, DWORD ItemID);
	
	//Cube: Splitting out the Approach Vendor message into its own function
	void 		ApproachVendor		(cClient *who);
	
	DWORD 		m_dwNPCModelID;
	
	//Cube: This is to add digit grouping to experience award messages.  There is probably a better way of doing this :p
	/*
	char* c_exp_reward(DWORD exp_r)
	{
		//Set up 
		char message[30];
		char comma[] = ",";
		
		itoa(exp_r,message,10);
		
		if (strlen(message) > 3 && strlen(message) < 6)
		{
			//copy a comma into the current string
			memcpy(message,comma,1);
		}
		if (strlen(message) > 6 && strlen(message) < 9)
		{
			memcpy(message,comma,1);
			memcpy(message,comma,1);
		}
		if (strlen(message) > 9)
		{
			memcpy(message,comma,1);
			memcpy(message,comma,1);
			memcpy(message,comma,1);
		}
		return message;
	}
	*/

	virtual ~cNPC( ) {}
	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	void	SetModel(WORD wNPCModel)
	{
		m_wNPCModel = wNPCModel;
	}
	void	SetNumMessages(int num)
	{
		iNumMessages = num;
	}
	int		GetNumMessages()
	{
		return iNumMessages;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}

	//Cubem0j0: Is NPC a vendor?
	void	SetIsVendor(int vend)
	{
		IsVendor = vend;
	}
	int		GetIsVendor()
	{
		return IsVendor;
	}

	//Cubem0j0: Get/Set npc_id variable
	void	Set_npc_id(int npcid)
	{
		npc_id = npcid;
	}
	int		Get_npc_id()
	{
		return npc_id;
	}

	void	SetString(std::string str,int index)
	{
		if(index<0||index>15)
		{
			return;
		}
		vsMessages[index] = str;
	}

	//Cubem0j0: Need a way to find the GUIDs of items in a vendors inventory
//	inline void SetVendorListID(DWORD ID)
//	{
//		vendor_item_ids.push_back(ID);
//	}

	//CubeM0j0:  Inventory stuff...

	inline void AddInventory( cObject *pcObject )
	{
		m_NPC_lstInventory.push_back( pcObject );
		pcObject->m_fIsOwned = TRUE;
		pcObject->m_dwContainer = m_dwGUID;
	}

	inline void RemoveInventory( cObject *pcObject )
	{
		iterObject_lst itObject = std::find( m_NPC_lstInventory.begin( ), m_NPC_lstInventory.end( ), pcObject );

		if ( itObject != m_NPC_lstInventory.end( ) )
			m_NPC_lstInventory.erase( itObject );
		pcObject->m_fIsOwned = FALSE;
		pcObject->m_dwContainer = NULL;
	}

	inline cObject *FindInventory( DWORD dwGUID )
	{
		for ( iterObject_lst itObject = m_NPC_lstInventory.begin( ); itObject != m_NPC_lstInventory.end( ); ++itObject )
		{
			if ( dwGUID == (*itObject)->GetGUID( ) )
				return *itObject;
		}
		return NULL;
	}

	inline void DeleteFromInventory( cObject *pcObject )
	{
		iterObject_lst itObject = std::find( m_NPC_lstInventory.begin( ), m_NPC_lstInventory.end( ), pcObject );

		if ( itObject != m_NPC_lstInventory.end( ) )
			m_NPC_lstInventory.erase( itObject );
	}

	cObjStats	m_NPCStats;
	DWORD		m_qitem_id1;

	//k109: Added this to try and get the items in vendor inventory to ID.
	DWORD		m_npc_target_id;

	//This may work better, an array for vendor item id's
	DWORD		v_guids[255];
	std::list< cNPC * > npc_ids;


private:
	int			npc_id;
	int			IsVendor;
	int			iNumMessages;
	WORD		m_wGender;
	std::string vsMessages[32];
	DWORD		m_dwMode;
	WORD		m_wNPCModel;
	DWORD		m_dwSellCategories;
	

	//Cubem0j0:  Add NPC Inventory...
	std::list< cObject * >	m_NPC_lstInventory;

	
};

class cLifestone : public cObject
{
public:
	cLifestone();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a lifestone object is initialized.
	 */
	cLifestone(WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc);
	virtual ~cLifestone() {}

	void		Assess	(cClient *pcAssesser );
	cMessage	CreatePacket();
	void		Action(cClient* who);
		void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}
	void	SetData(WORD wAnimConfig, WORD wSoundSet,WORD wModel,WORD wIcon, DWORD dwObject1, DWORD dwObject2)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wSoundSet		=	wSoundSet;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
		m_dwObjectsFlag1	=	dwObject1;
		m_dwObjectsFlag2	=	dwObject2;
	}
private:

	DWORD		m_dwMode;
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wSoundSet;
	WORD		m_wPortalMode;
	WORD		m_wAnimConfig;
	DWORD		m_dwObjectsFlag1;
	DWORD		m_dwObjectsFlag2;

};

class cPortal : public cObject
{
public:
	cPortal		( );
	/**
	 *	A constructor.
	 *
	 *	Called whenever a portal object is initialized.
	 */
	cPortal		( DWORD dwGUID,  DWORD dwColor, cLocation *Loc, cLocation *destLoc, char *szName, char *szDescription, DWORD dwLowerRestrict, DWORD dwHigherRestrict );
	//cPortal		( DWORD dwGUID, cLocation& Loc, DOUBLE dNS, DOUBLE dEW, char *szName, DWORD dwLowerRestrict, DWORD dwHigherRestrict );
	virtual ~cPortal	( ) {}	
	
	void		Assess		( cClient *pcAssesser );
	void		Use			( cClient *pcUser );
	void		Action		( cClient *who );
	cMessage	CreatePacket( ); 

	inline void SetDestination( cLocation *pcLoc )
	{
		m_cDestination.m_dwLandBlock	= pcLoc->m_dwLandBlock;
		m_cDestination.m_flX			= pcLoc->m_flX;
		m_cDestination.m_flY			= pcLoc->m_flY;
		m_cDestination.m_flZ			= pcLoc->m_flZ;
		m_cDestination.m_flA			= pcLoc->m_flA;
		m_cDestination.m_flB			= pcLoc->m_flB;
		m_cDestination.m_flC			= pcLoc->m_flC;
		m_cDestination.m_flW			= pcLoc->m_flW;
	}
private:
	cLocation	m_cWorldLoc;
	cLocation	m_cDestination;
	//DWORD		m_dwLandblock;
	//float		m_flX;
	//float		m_flY;
	//float		m_flZ;
	//float		m_fHeading;
	DWORD		m_dwLowerRestriction;
	DWORD		m_dwHigherRestriction;
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wPortalMode;
	WORD		m_wAnimConfig;
	DWORD		m_dwObjectsFlag1;
	DWORD		m_dwObjectsFlag2;
};

class cAbiotic : public cObject
{
public:
	cAbiotic( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwWeight, DWORD dwValue, BOOL fSelectable = TRUE, BOOL fEquippable = TRUE, BOOL fStackable = FALSE, BOOL fIsOwned = FALSE, DWORD dwContainer = 0 )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_dwWeight			= dwWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= FALSE;
		m_wState			= 0;
		m_dwDoorState		= 0x0CL;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;

		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	cAbiotic( ) {}
	virtual ~cAbiotic( ) {}

	cMessage	CreatePacket	( );
	cMessage	Animation		( WORD wAnim, float flPlaySpeed ) { return *((cMessage *)NULL); }
	void		Assess			( cClient *pcAssesser );
	void		Use				( cClient *pcAssesser )	{}
	void		Action			( cClient *pcClient )	{}

	void SetContainer( DWORD dwGUID );

protected:
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  ITEM CLASSES
*
*	These classes define items.
************************************/

/************************************
*	Cubem0j0:  item class - Misc
************************************/
class cMisc : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a miscellaneous object is initialized in the world.
	 */
	cMisc( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a miscellaneous object is initialized in a container.
	 */
	cMisc( DWORD dwGUID, DWORD Container, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;	
	}

	cMisc( ) {}
	virtual ~cMisc( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }

	DWORD		m_dwItemModelID;

protected:
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Food
************************************/
class cFood : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a food object is initialized in the world.
	 */
	cFood( DWORD dwGUID, cLocation& Loc, DWORD dwModel, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wBurden, DWORD dwVitalID, DWORD dwAmount )  : m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wBurden			= wBurden;
		m_dwValue			= dwValue;
		m_dwVitalID			= dwVitalID; 
		m_dwAmount			= dwAmount;	
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a food object is initialized in a container.
	 */
	cFood( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wStack, WORD wBurden, DWORD dwVitalID, DWORD dwVitalAffect)  : m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wBurden			= wBurden;
		m_dwValue			= dwValue;
		m_dwVitalID			= dwVitalID; 
		m_wStack			= wStack;
		m_dwContainer		= dwContainer;
		m_dwVitalAffect		= dwVitalAffect;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}

	cFood( ) {}
	virtual ~cFood( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	cMessage	Animation		( WORD wAnim, float flPlaySpeed ) { return *((cMessage *)NULL); }
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	
	DWORD		m_dwItemModelID;

protected:
	DWORD		m_dwValue;
	WORD		m_wBurden;
	DWORD		m_dwVitalID;
	WORD		m_wStack;
	DWORD		m_dwVitalAffect;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Armor
************************************/
class cArmor : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever an armor object is initialized in the world.
	 */
	cArmor( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
	//	m_wWeight			= wWeight;
	//	m_dwValue			= dwValue;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		m_bWearPaletteChange= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever an armor object is initialized in a container.
	 */
	cArmor( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wBurden, DWORD dwArmor_Level, float fProt_Slashing, float fProt_Piercing, float fProt_Bludgeon, float fProt_Fire, float fProt_Cold, float fProt_Acid, float fProt_Electric)  :	m_dwItemModelID( dwModel )
		//BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
//		m_fSelectable		= fSelectable;
		m_wBurden			= wBurden;
		m_dwValue			= dwValue;
//		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
//		m_fEquippable		= fEquippable;
//		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
		m_dwArmor_Level		= dwArmor_Level;
		m_fProt_Slashing	= fProt_Slashing;
		m_fProt_Piercing	= fProt_Piercing;
		m_fProt_Bludgeon	= fProt_Bludgeon;
		m_fProt_Fire		= fProt_Fire;
		m_fProt_Cold		= fProt_Cold;
		m_fProt_Acid		= fProt_Acid;
		m_fProt_Electric	= fProt_Electric;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		m_bWearPaletteChange= 0;
	}
	cArmor( ) {}
	virtual ~cArmor( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }

	inline	DWORD	GetSlashProt	( ) { return m_fProt_Slashing; }

	DWORD		m_dwItemModelID;

protected:
	DWORD		m_dwValue;
	WORD		m_wBurden;

	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	DWORD		m_dwActivate_Req_SkillID;
	DWORD		m_dwActivate_Req_Skill_Level;
	DWORD		m_dwArmor_Level;
	float		m_fProt_Slashing;
	float		m_fProt_Piercing;
	float		m_fProt_Bludgeon;
	float		m_fProt_Fire;
	float		m_fProt_Cold;
	float		m_fProt_Acid;
	float		m_fProt_Electric;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Scrolls
************************************/
class cScrolls : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a scroll object is initialized in the world.
	 */
	cScrolls( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a scroll object is initialized in a container.
	 */
	cScrolls( DWORD dwGUID, DWORD Container, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;	
	}

	cScrolls( ) {}
	virtual ~cScrolls( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }

	DWORD		m_dwItemModelID;

protected:
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Healing Kits
************************************/
class cHealingKits : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a healing kit object is initialized.
	 */
	cHealingKits( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, WORD wUses, WORD wUseLimit)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		dwValue				= m_dwValue;
		wWeight				= m_wWeight;
		m_wUses				= wUses;
		m_wUseLimit			= wUseLimit;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;

		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	cHealingKits( ) {}
	virtual ~cHealingKits( ) {}

	cMessage	CreatePacket	( );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	DWORD		m_wUses;
	DWORD		m_wUseLimit;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Gems
************************************/
class cGems : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a gem object is initialized in the world.
	 */
	cGems( DWORD dwGUID, cLocation& Loc, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a gem object is initialized in a container.
	 */
	cGems( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_dwValue			= dwValue;
		m_wWeight			= wWeight;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	
	cGems( ) {}
	virtual ~cGems( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;
	
protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Books
************************************/
class cBooks : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a book object is initialized in the world.
	 */
	cBooks( DWORD dwGUID, cLocation& Loc, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a book object is initialized in a container.
	 */
	cBooks( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wBurden) :	m_dwItemModelID( dwModel )
		
		/*
		, DWORD ContentPages, DWORD UsedPages, DWORD TotalPages, 
		std::string strAuthor, std::string strTitle, std::string strComment, std::string strCommentAuthor, std::string strPage1, std::string strPage2, std::string strPage3, std::string strPage4, std::string strPage5,
		std::string strPage6, std::string strPage7, std::string strPage8, std::string strPage9)  :	m_dwItemModelID( dwModel )
		*/
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wBurden			= wBurden;
		m_dwValue			= dwValue;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	
	cBooks( ) {}
	virtual ~cBooks( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );
	void		Read			( cClient* who, DWORD GUID, DWORD PageNumber );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;
	


protected:
	WORD		m_wBurden;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;
	



	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Manastones
************************************/
class cManaStones : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a mana stone object is initialized in the world.
	 */
	cManaStones( DWORD dwGUID, cLocation& Loc, DWORD dwModel, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a mana stone object is initialized in a container.
	 */
	cManaStones( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_dwContainer		= dwContainer;

//		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x0;
		m_fEquipped			= 0;
	}
	cManaStones( ) {}
	virtual ~cManaStones( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Lockpicks
************************************/
class cLockpicks : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a lockpick object is initialized in the world.
	 */
	cLockpicks( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a lockpick object is initialized in a container.
	 */
	cLockpicks( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, WORD wUses, WORD wUseLimit)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_dwContainer		= dwContainer;
		m_wUses				= wUses;
		m_wUseLimit			= wUseLimit;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;	
	}
	cLockpicks( ) {}
	virtual ~cLockpicks( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;
	WORD		m_wUses;
	WORD		m_wUseLimit;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - TradeSkillMats
************************************/
class cTradeSkillMats : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a trade skill material object is initialized in the world.
	 */
	cTradeSkillMats( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a trade skill material object is initialized in a container.
	 */
	cTradeSkillMats( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	cTradeSkillMats( ) {}
	virtual ~cTradeSkillMats( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Casting Items (Wands, Orbs, etc)
************************************/
class cWands : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a casting object is initialized in the world.
	 */
	cWands( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;

		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a casting object is initialized in a container.
	 */
	cWands( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	cWands( ) {}
	virtual ~cWands( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Spell Components
************************************/
class cSpellComps : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a spell component object is initialized in the world.
	 */
	cSpellComps( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a spell component object is initialized in a container.
	 */
	cSpellComps( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wStack, WORD wWeight )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_dwContainer		= dwContainer;
		m_wStack			= wStack;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}

	cSpellComps( ) {}
	virtual ~cSpellComps( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	WORD		m_wStack;
	DWORD		m_dwValue;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Plants
************************************/
class cPlants : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a plant object is initialized in the world.
	 */
	cPlants( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a plant object is initialized a container.
	 */
	cPlants( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, DWORD wWeight )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_dwContainer		= dwContainer;

		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}

	cPlants( ) {}
	virtual ~cPlants( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Ammunition
************************************/
class cAmmo : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever an ammunition object is initialized in the world.
	 */
	cAmmo( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever an ammunition object is initialized in a container.
	 */
	cAmmo( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, WORD wStack, WORD wStackLimit )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_dwContainer		= dwContainer;
		m_wStack			= wStack;
		m_wStackLimit		= wStackLimit;
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	cAmmo( ) {}
	virtual ~cAmmo( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;
	WORD		m_wStack;
	WORD		m_wStackLimit;

	std::string	m_strDescription;
};

/************************************
 * Cubem0j0: item class - Weapon
 ************************************/
 class cWeapon : public cObject
 {
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a weapon object is initialized in the world.
	 */
	cWeapon( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, 
		DWORD dwValue, WORD wWeight, BYTE bWieldType, DWORD dwIconHighlight, float fWorkmanship, DWORD dwMaterialType, DWORD dwWeaponDamage, 
		DWORD dwWeaponSpeed, DWORD dwWeaponSkill, DWORD dwDamageType, double dWeaponVariance, double dWeaponModifier, double dWeaponPower, 
		double dWeaponAttack)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_bWieldType		= bWieldType;
		m_wIconHighlight	= dwIconHighlight;
		m_fWorkmanShip		= fWorkmanship;
		m_dwMaterialType	= dwMaterialType;
		m_dwWeaponDamage	= dwWeaponDamage;
		m_dwWeaponSpeed		= dwWeaponSpeed;
		m_dwWeaponSkill		= dwWeaponSkill;
		m_dwDamageType		= dwDamageType;
		m_dWeaponVariance	= dWeaponVariance;
		m_dWeaponModifier	= dWeaponModifier;
		m_dWeaponPower		= dWeaponPower;
		m_dWeaponAttack		= dWeaponAttack;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
 
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a weapon object is initialized in a container.
	 */
	cWeapon( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, 
		DWORD dwValue, WORD wWeight, BYTE bWieldType, DWORD dwIconHighlight, float fWorkmanship, DWORD dwMaterialType, DWORD dwWeaponDamage, 
		DWORD dwWeaponSpeed, DWORD dwWeaponSkill, DWORD dwDamageType, double dWeaponVariance, double dWeaponModifier, double dWeaponPower, 
		double dWeaponAttack )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
//		m_fSelectable		= fSelectable;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_bWieldType		= bWieldType;
		m_wIconHighlight	= dwIconHighlight;
		m_fWorkmanShip		= fWorkmanship;
		m_dwMaterialType	= dwMaterialType;
		m_dwWeaponDamage	= dwWeaponDamage;
		m_dwWeaponSpeed		= dwWeaponSpeed;
		m_dwWeaponSkill		= dwWeaponSkill;
		m_dwDamageType		= dwDamageType;

		m_dWeaponVariance	= dWeaponVariance;
		m_dWeaponModifier	= dWeaponModifier;
		m_dWeaponPower		= dWeaponPower;
		m_dWeaponAttack		= dWeaponAttack;
//		m_fIsOwned			= fIsOwned;
//		m_dwContainer		= dwContainer;
//		m_fEquippable		= fEquippable;
//		m_fIsStackable		= fStackable;
//		m_fIsSolid			= fSolid;
//		m_fIsOwned			= TRUE;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	 cWeapon( ) {}
	 virtual ~cWeapon( ) {}
 
	 cMessage CreatePacket ( );
	 cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	 void Assess ( cClient *pcAssesser );
	 void Action ( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	
	inline  DWORD	GetItemValue	( ) { return m_dwValue; }

	int GetIsUAWeapon()
	{
		return isUAWeapon;
	}

	DWORD		m_dwItemModelID;
	BYTE		m_bWieldType;
	int			isUAWeapon;
	DWORD	m_dwDamageType;

protected:
	WORD	m_wWeight;
	DWORD	m_dwValue;

	DWORD	m_wIconHighlight;
	float	m_fWorkmanShip;
	DWORD	m_dwMaterialType;

	DWORD	m_dwWeaponDamage;
	DWORD	m_dwWeaponSpeed;
	DWORD	m_dwWeaponSkill;


	double	m_dWeaponVariance;
	double	m_dWeaponModifier;
	double	m_dWeaponPower;
	double	m_dWeaponAttack;

	BOOL	m_fIsStackable;
	BOOL	m_fEquippable;
	BOOL	m_fIsSolid;
 
	std::string m_strDescription;
 };
 
/************************************
*	Cubem0j0:  item class - Salvage
************************************/
class cSalvage : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a salvage object is initialized in the world.
	 */
	cSalvage( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a salvage object is initialized in a container.
	 */
	cSalvage( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	
	cSalvage( ) {}
	virtual ~cSalvage( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	
	DWORD		m_dwItemModelID;
	
protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Pyreals
************************************/
class cPyreals : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a pyreal object is initialized in the world.
	 */
	cPyreals( DWORD dwGUID, cLocation& Loc, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wStack, WORD wStackLimit)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;

		m_strName			= strName;
		m_strDescription	= strDescription;
		m_dwValue			= dwValue;
		m_wStack			= wStack;
		m_wStackLimit		= wStackLimit;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a pyreal object is initialized in a container.
	 */
	cPyreals( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wStack, WORD wStackLimit)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_dwValue			= dwValue;
		m_wStack			= wStack;
		m_wStackLimit		= wStackLimit;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}

	cPyreals( ) {}
	virtual ~cPyreals( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );
	void		Stack			( cClient* who, DWORD item1, DWORD item2, DWORD amount);
	void		Split			( cClient* who, DWORD item_guid, DWORD slot, DWORD value );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

	WORD	GetStack()
	{
		return m_wStack;
	}

	WORD	SetStack(WORD amount)
	{
		m_wStack = amount;
	}

protected:
	WORD		m_wStack;
	WORD		m_wStackLimit;
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Trade Notes
************************************/
class cTradeNotes : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a trade note object is initialized in the world.
	 */
	cTradeNotes( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a trade note object is initialized in a container.
	 */
	cTradeNotes( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}

	cTradeNotes( ) {}
	virtual ~cTradeNotes( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Jewelry
************************************/
class cJewelry : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a jewelry object is initialized in the world.
	 */
	cJewelry( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;

		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a jewelry object is initialized in a container.
	 */
	cJewelry( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	
	cJewelry( ) {}
	virtual ~cJewelry( ) {}

	cMessage	CreatePacket	( );
	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;
	
protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Foci
************************************/
class cFoci : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a foci object is initialized in the world.
	 */
	cFoci( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
	//	m_wWeight			= wWeight;
	//	m_dwValue			= dwValue;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;

		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a foci object is initialized in a container.
	 */
	cFoci( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	cFoci( ) {}
	virtual ~cFoci( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Packs
************************************/
class cPack : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a pack object is initialized in the world.
	 */
	cPack( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
	//	m_wWeight			= wWeight;
	//	m_dwValue			= dwValue;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a pack object is initialized in a container.
	 */
	cPack( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	
	cPack( ) {}
	virtual ~cPack( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Clothes
************************************/
class cClothes : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a clothing object is initialized in the world.
	 */
	cClothes( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
	//	m_wWeight			= wWeight;
	//	m_dwValue			= dwValue;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		m_bWearPaletteChange= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a clothing object is initialized in a container.
	 */
	cClothes( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wWeight, BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_wWeight			= wWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
//		m_dwArmorLevel		= dwArmor_Level;
//		m_fProt_Slashing	= fProt_Slashing;
//		m_fProt_Piercing	= fProt_Piercing;
//		m_fProt_Bludgeon	= fProt_Bludgeon;
//		m_fProt_Fire		= fProt_Fire;
//		m_fProt_Cold		= fProt_Cold;
//		m_fProt_Acid		= fProt_Acid;
//		m_fProt_Electric	= fProt_Electric;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		m_bWearPaletteChange= 0;
	}
	cClothes( ) {}
	virtual ~cClothes( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	WORD		m_wWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	DWORD		m_dwActivate_Req_SkillID;
	DWORD		m_dwActivate_Req_Skill_Level;
	DWORD		m_dwArmor_Level;
	float		m_fProt_Slashing;
	float		m_fProt_Piercing;
	float		m_fProt_Bludgeon;
	float		m_fProt_Fire;
	float		m_fProt_Cold;
	float		m_fProt_Acid;
	float		m_fProt_Electric;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  item class - Shields
************************************/
class cShield : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a shield object is initialized the world.
	 */
	cShield( DWORD dwGUID, cLocation& Loc, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wBurden, DWORD ArmorLevel)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wBurden			= wBurden;
		m_dwValue			= dwValue;
		m_dwArmorLevel		= ArmorLevel;
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	/**
	 *	A constructor.
	 *
	 *	Called whenever a shield object is initialized in a container.
	 */
	cShield( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, WORD wIcon, std::string strName, std::string strDescription, DWORD dwValue, WORD wBurden, DWORD ArmorLevel)  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_wIcon				= wIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_wBurden			= wBurden;
		m_dwValue			= dwValue;
		m_dwContainer		= dwContainer;
		m_dwArmorLevel		= ArmorLevel;

		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	
	cShield( ) {}
	virtual ~cShield( ) {}

	cMessage	CreatePacket	( );
	cMessage CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;
	DWORD		m_dwArmorLevel;
protected:
	WORD		m_wBurden;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

/************************************
*	Cubem0j0:  Container Classes
*
*	These classes spawn items in containers
*	I needed these for healing kits, etc.
*	Anything that uses the 0x0035 as its use
*	message.
************************************/
/************************************
*	Cubem0j0:  cHealingCon
************************************/
class cHealingCon : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a healing container object is initialized in the world.
	 */
	cHealingCon( DWORD dwGUID, DWORD dwContainer, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwWeight, DWORD dwValue, WORD wUses, WORD wUseLimit, BOOL fSelectable = TRUE, BOOL fEquippable = FALSE, BOOL fStackable = TRUE, BOOL fIsOwned = TRUE )  :	m_dwItemModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon				= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_dwWeight			= dwWeight;
		m_dwValue			= dwValue;
		m_wUses				= wUses;
		m_wUseLimit			= wUseLimit;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
		m_fIsOwned			= TRUE;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
	}
	cHealingCon( ) {}
	virtual ~cHealingCon( ) {}

	cMessage	CreatePacket();

	cMessage	CreatePacketContainer	( DWORD Container, DWORD ItemModelID );
	cMessage	Animation		( WORD wAnim, float flPlaySpeed ) { return *((cMessage *)NULL); }
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	void SetContainer( DWORD dwGUID );

	inline	DWORD	GetItemModelID	( ) { return m_dwItemModelID; }
	inline  DWORD	GetContainerID  ( ) { return m_dwContainer; }
	DWORD		m_dwItemModelID;

protected:
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	WORD		m_wUses;
	WORD		m_wUseLimit;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;
	
	std::string	m_strDescription;
};

class cEquipment
{
public:
	cArmor*		ca_Shield;
	cArmor*		ca_Helmet;
	cArmor*		ca_UpperArm;
	cArmor*		ca_LowerArm;
	cArmor*		ca_Breastplate;
	cArmor*		ca_Girth;
	cArmor*		ca_UpperLegs;
	cArmor*		ca_LowerLegs;
//	cJewerly*	cj_Neck;
//	cJewerly*	cj_LeftRing;
//	cJewerly*	cj_RightRing;
//	cJewerly*	cj_LeftBracelet;
//	cJewerly*	cj_RightBracelet;
	cWeapon*	cw_Weapon;
};

class cInventory
{
public:
	std::vector< cObject * > m_vInventoryObjects;
	cEquipment				*m_cEquipment;
};

#define PK_ALTAR		1
#define NPK_ALTAR		0
//857
//846

//************************************
//*	Object class - Altars
//************************************
class cAltar: public cObject
{
public:
	cAltar	();
	/**
	 *	A constructor.
	 *
	 *	Called whenever an altar object is initialized.
	 */
	cAltar	(DWORD dwType, DWORD dwGUID, cLocation *pcLoc, char* strName, char* strDesc);// :
	/*cAbiotic( dwGUID, *pcLoc,(type ? 857 : 846), 1, TRUE, 2323, 
		  (type ? "Altar to Bael'Zharon" : "Altar to Asheron"), 
		  (type ? "Double click to go PK" : "Double Click to go NPK")
		  , 1000, 1000, TRUE, FALSE, FALSE, FALSE), dwType(type ? 1:0)
	{
		m_fStatic = TRUE;
	}
	*/
	virtual ~cAltar	(){}
	  void		Assess	(cClient *pcAssesser );
	  cMessage	CreatePacket();
	  void      Action(cClient* who);
	  
	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}
	void	SetData(WORD wAnimConfig, WORD wSoundSet,WORD wModel,WORD wIcon, DWORD dwObject)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wSoundSet		=	wSoundSet;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}
private:
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wSoundSet;
	WORD		m_wAnimConfig;
	DWORD		dwType; //0 = NPK 1 = PK  
};

#define DOOR_OPEN		1
#define DOOR_CLOSED		0

//************************************
//*	Object class - Doors
//************************************
class cDoor : public cObject
{
	friend class cClient;
public:
	cDoor		();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a door object is initialized.
	 */
	cDoor		( WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc );
	virtual ~cDoor		( ) {}	
	
	void		Assess		( cClient *pcAssesser );
	void		Action		(cClient* who);
	cMessage	CreatePacket( ); 

	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}
	void	SetData(WORD wAnimConfig, WORD wSoundSet,WORD wModel,WORD wIcon)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wSoundSet		=	wSoundSet;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}
private:

	DWORD		m_dwMode;
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wSoundSet;
	WORD		m_wPortalMode;
	WORD		m_wAnimConfig;
};

//************************************
//*	Object class - World Objects (town signs, wells, etc)
//************************************
class cWorldObject : public cObject
{
public:
	cWorldObject();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a world object is initialized.
	 */
	cWorldObject(WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc);
	virtual ~cWorldObject() {}

	void		Assess	(cClient *pcAssesser );
	cMessage	CreatePacket();
	void		Action(cClient* who);
	
//	 * Cubem0j0:  Set the type of object
//	 * 0 = Unselectable
//	 * 1 = Selectable
//	 * 2 = Well/Fountain

	void	SetObjectType(int ot)
	{
		iObjectType = ot;
	}
	int		GetObjectType()
	{
		return iObjectType;
	}

	void SetData(WORD wModel,WORD wIcon)	

	{
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}
private:
	int	iObjectType;
	DWORD		m_dwMode;
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wPortalMode;
};

//************************************
//*	Object class - Merchant Signs
//************************************
class cMerchantSign : public cObject
{
public:
	/**
	 *	A constructor.
	 *
	 *	Called whenever a merchant sign object is initialized.
	 */
	cMerchantSign( DWORD dwGUID, cLocation& Loc, DWORD dwModel, float flScale, BOOL fSolid, DWORD dwIcon, std::string strName, std::string strDescription, DWORD dwWeight, DWORD dwValue, BOOL fSelectable = TRUE, BOOL fEquippable = TRUE, BOOL fStackable = FALSE, BOOL fIsOwned = FALSE, DWORD dwContainer = 0 )  :	m_dwWOModelID( dwModel )
	{
		m_dwGUID			= dwGUID;
		m_dwModel			= dwModel;
		m_flScale			= flScale;
		m_wIcon			= dwIcon;
		m_strName			= strName;
		m_strDescription	= strDescription;
		m_fSelectable		= fSelectable;
		m_dwWeight			= dwWeight;
		m_dwValue			= dwValue;
		m_fIsOwned			= fIsOwned;
		m_dwContainer		= dwContainer;
		m_fEquippable		= fEquippable;
		m_fIsStackable		= fStackable;
		m_fIsSolid			= fSolid;
//		m_fIsOwned			= FALSE;
//		m_wState			= 0;
//		m_dwDoorState		= 0x0CL;
		
		m_bInventorySequence= -1;
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x3C;
		m_fEquipped			= 0;
		
		CopyMemory( &m_Location, &Loc, sizeof( cLocation ) );
	}
	cMerchantSign( ) {}
	virtual ~cMerchantSign( ) {}

	cMessage	CreatePacket	( );
	cMessage	Animation		( WORD wAnim, float flPlaySpeed ) { return *((cMessage *)NULL); }
	void		Assess			( cClient *pcAssesser );
	void		Action			( cClient* who );

	//	void SetContainer( DWORD dwGUID );

	inline	DWORD	GetWorldObject	( ) { return m_dwWOModelID; }

	void	SetObjectType(int ot)
	{
		iObjectType = ot;
	}
	int		GetObjectType()
	{
		return iObjectType;
	}

protected:
	int			iObjectType;
	DWORD		m_dwWOModelID;
	DWORD		m_dwWeight;
	DWORD		m_dwValue;
	BOOL		m_fIsStackable;
	BOOL		m_fEquippable;
	BOOL		m_fIsSolid;

	std::string	m_strDescription;
};

//************************************
//*	World objects - Chest Containers
//************************************
class cChest : public cObject
{
	friend class cClient;
public:
	cChest		();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a chest object is initialized.
	 */
	cChest		( WORD type, DWORD dwGUID, cLocation *pcLoc, char *szName, char *szDesc );
	virtual ~cChest		( ) {}	
	
	void		Assess		( cClient *pcAssesser );
	void		Action		(cClient* who);
	cMessage	CreatePacket( ); 

	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}

	//Cubem0j0:  Set locked status based on database
	void	SetIsLocked(int locked)
	{
		IsLocked = locked;
	}
	int		GetIsLocked()
	{
		return IsLocked;
	}

	//Cubem0j0:  If this object is locked, how hard is it to open.
	void	SetLDiff(int ldiff)
	{
		LockDiff = ldiff;
	}
	int		GetLDiff()
	{
		return LockDiff;
	}

	void	SetData(WORD wAnimConfig, WORD wSoundSet,WORD wModel,WORD wIcon, DWORD dwObject1, DWORD dwObject2)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wSoundSet		=	wSoundSet;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
		m_dwObjectsFlag1	=	dwObject1;
		m_dwObjectsFlag2    =   dwObject2;
	}
private:

	DWORD		m_dwMode;
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wSoundSet;
	WORD		m_wPortalMode;
	WORD		m_wAnimConfig;
	DWORD		m_dwObjectsFlag1;
	DWORD		m_dwObjectsFlag2;
	int			IsLocked;
	int			LockDiff;
};

//************************************
//*	World objects - Corpse Containers
//************************************
class cCorpse : public cObject
{
	friend class cClient;
	friend class cMonster;

public:
	cCorpse		() {};
	/**
	 *	A constructor.
	 *
	 *	Called whenever a corpse object is initialized.
	 */
	cCorpse		( WORD type, DWORD dwGUID, cLocation& Loc, char *szName, char *szDesc );
	virtual ~cCorpse		( ) {}	
	
	void		Assess			( cClient *pcAssesser );
	void		Action			(cClient* who);
	cMessage	CreatePacket	( );

	//Returns the GUID of the object
	inline DWORD 	GetCorpseGUID( ) 
	{ 
		return m_dwGUID; 
	}
	
	//Cube:  This is the MonsterModelID for the corpse.
	inline DWORD 	GetType()
	{
		return m_dwType;
	}
		
	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}
	void	SetData(WORD wAnimConfig, WORD wSoundSet,WORD wModel,WORD wIcon, DWORD MonsterModelID, float flScale)
	{
		m_wAnimConfig		=	wAnimConfig;
		m_wSoundSet			=	wSoundSet;
		m_wModel			=	wModel;
		m_wIcon				=	wIcon;
		m_MonsterModelID	=	MonsterModelID;
		m_flScale			=   flScale;
	}

	//CubeM0j0:  Inventory stuff...

	inline void AddInventory( cObject *pcObject )
	{
		m_cor_lstInventory.push_back( pcObject );
		pcObject->m_fIsOwned = TRUE;
		pcObject->m_dwContainer = m_dwGUID;
	}

	inline void RemoveInventory( cObject *pcObject )
	{
		iterObject_lst itObject = std::find( m_cor_lstInventory.begin( ), m_cor_lstInventory.end( ), pcObject );

		if ( itObject != m_cor_lstInventory.end( ) )
			m_cor_lstInventory.erase( itObject );
		pcObject->m_fIsOwned = FALSE;
		pcObject->m_dwContainer = NULL;
	}

	inline cObject *FindInventory( DWORD dwGUID )
	{
		for ( iterObject_lst itObject = m_cor_lstInventory.begin( ); itObject != m_cor_lstInventory.end( ); ++itObject )
		{
			if ( dwGUID == (*itObject)->GetGUID( ) )
				return *itObject;
		}
		return NULL;
	}

	inline void DeleteFromInventory( cObject *pcObject )
	{
		iterObject_lst itObject = std::find( m_cor_lstInventory.begin( ), m_cor_lstInventory.end( ), pcObject );

		if ( itObject != m_cor_lstInventory.end( ) )
			m_cor_lstInventory.erase( itObject );
	}

	//Cubem0j0:  Add Corpse Inventory...
	
private:

	DWORD		m_dwMode;
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wSoundSet;
	WORD		m_wPortalMode;
	WORD		m_wAnimConfig;
	DWORD		m_MonsterModelID;
	float		m_flScale;
	
	//the corpse's inventory list
	std::list< cObject * >	m_cor_lstInventory;
};

//************************************
//*	World objects - Covenant Crystals
//************************************
class cCovenant: public cObject
{
	friend class cClient;
public:
	cCovenant();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a covenant crystal object is initialized.
	 */
	cCovenant( WORD type, DWORD dwGUID, DWORD houseID, char *szName, char *szDescription, cLocation *pcLoc = NULL );

	void	  Assess	(cClient *pcAssesser );
	cMessage  CreatePacket( );
	void      Action(cClient* who);

	virtual ~cCovenant( ) {}

	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	void	SetGUID(DWORD GUID)
	{
		m_dwGUID = GUID;
	}
	void	SetOwner(DWORD ownerID)
	{
		m_dwOwnerID = ownerID;
	}
	void	SetData(DWORD wAnimConfig,WORD wModel,WORD wIcon)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}

private:
	DWORD	m_dwType;		//0 = Open, 1 = Closed
	DWORD	m_dwHouseID;
	WORD	m_wModel;
	WORD	m_wIcon;
	WORD	m_wAnimConfig;
	DWORD	m_dwHouseType;
};

#define HOUSE_FORSALE		1
#define HOUSE_OWNED			2
#define HOUSE_OPEN			3
#define HOUSE_CLOSED		4

//************************************
//*	World objects - House Objects
//************************************
class cHouse : public cObject
{
	friend class cClient;

public:
	cHouse();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a house object is initialized.
	 */
	cHouse( char *szName, char *szDescription, WORD type, DWORD GUID, cLocation *pcLoc = NULL );
	virtual ~cHouse( ) {}

	void		cHouse::Assess	( cClient *pcAssesser );
	cMessage	cHouse::CreatePacket();
	void		cHouse::Action(cClient* who);

	void	SetGUID(DWORD GUID)
	{
		m_dwGUID = GUID;
	}
	void	SetOwner(DWORD ownerID)
	{
		m_dwOwnerID = ownerID;
	}
	void	SetData(WORD wModel,WORD wIcon)
	{
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}
		void	SetIsOpen(DWORD open)
	{
		IsOpen	=	open;
	}

private:
	WORD	m_wModel;
	WORD	m_wIcon;
	DWORD	IsOpen;

};

//************************************
//*	World objects - Housing Hooks
//************************************
class cHooks: public cObject
{
	friend class cClient;
public:
	cHooks();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a housing hook object is initialized.
	 */
	cHooks( WORD type, DWORD dwGUID, DWORD houseID, char *szName, char *szDescription, cLocation *pcLoc = NULL );

	void	  Assess	(cClient *pcAssesser );
	cMessage  CreatePacket( );
	void      Action(cClient* who);

	virtual ~cHooks( ) {}

	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	void	SetGUID(DWORD GUID)
	{
		m_dwGUID = GUID;
	}

	void	Set_hook_id(int hookid)
	{
		hook_id = hookid;
	}
	int		Get_hook_id()
	{
		return hook_id;
	}
/*
	inline void SetItem( cObject *pcObject ) 
	{ 
		m_hook_item = pcObject;
		m_hook_item->m_fIsOwned = TRUE;
		m_hook_item->m_dwContainer = m_dwGUID; 
	}

	inline void RemoveItem()
	{
		m_hook_item = NULL;
		m_hook_item->m_fIsOwned = FALSE;
		m_hook_item->m_dwContainer = NULL;
	}

	void	SetIsUsed(int used)
	{
		IsUsed = used;
	}
	int		GetIsUsed()
	{
		return IsUsed;
	}
*/
	void	SetData(WORD wAnimConfig,WORD wSoundSet,WORD wModel,WORD wIcon)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wSoundSet		=	wSoundSet;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}

	DWORD	v_guids[255];
	DWORD	m_dwHouseID;

private:
	int		IsUsed;
	DWORD	m_dwType;
	WORD	m_wModel;
	WORD	m_wIcon;
	WORD	m_wSoundSet;
	WORD	m_wAnimConfig;
	int		hook_id;

//	cObject	*m_hook_item;
//	DWORD	hook_item_id;
};

//************************************
//*	World objects - Housing Storage Chests
//************************************
class cStorage: public cObject
{
	friend class cClient;
public:
	cStorage();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a housing storage chest object is initialized.
	 */
	cStorage( WORD type, DWORD dwGUID, DWORD houseID, char *szName, char *szDescription, cLocation *pcLoc = NULL );

	void	  Assess	(cClient *pcAssesser );
	cMessage  CreatePacket( );
	void      Action(cClient* who);

	virtual ~cStorage( ) {}

	void	SetMode(DWORD mode)
	{
		m_dwMode = mode;
	}
	DWORD	GetMode()
	{
		return m_dwMode;
	}
	void	SetGUID(DWORD GUID)
	{
		m_dwGUID = GUID;
	}
	void	SetData(WORD wAnimConfig,WORD wSoundSet,WORD wModel,WORD wIcon)
	{
		m_wAnimConfig	=	wAnimConfig;
		m_wSoundSet		=	wSoundSet;
		m_wModel		=	wModel;
		m_wIcon			=	wIcon;
	}
	DWORD	m_dwHouseID;

private:
	DWORD		m_dwType;
	WORD		m_wModel;
	WORD		m_wIcon;
	WORD		m_wSoundSet;
	WORD		m_wPortalMode;
	WORD		m_wAnimConfig;
};

//************************************
//*	World objects - War Spells
//************************************
class cWarSpell : public cObject  
{
	friend class cClient;

public:
	//WarSpell();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a warspell object is initialized.
	 */
	cWarSpell(DWORD dwGUID, DWORD dwSpellID, cLocation& Loc, cVelocity tarVel, DWORD dwSpellModel)
	{
		m_wPositionSequence = 0;
		m_wNumPortals		= 0;
		m_wNumLogins		= 0x1;
		m_fStatic			= FALSE;	
		SetLocation(Loc);
		m_dwGUID			= dwGUID;
		m_dwSpellModel		= dwSpellModel;
		m_dwSpellID			= dwSpellID;
		m_strName			= "War Spell";
		m_Velocity			= tarVel;
		m_Location.m_flX    += tarVel.m_dx;
		m_Location.m_flY    += tarVel.m_dy;
		m_Location.m_flZ    += 1.4f + tarVel.m_dz;
		
	};
	virtual ~cWarSpell(){};

	cMessage	cWarSpell::CreatePacket();
	void		cWarSpell::Assess	(cClient *pcAssesser );
	void		cWarSpell::Action(cClient* who);
	static int	cWarSpell::Move( LPVOID wp, LPVOID lp );
	cMessage	cWarSpell::WarParticle(cWarSpell* pcWarSpell,DWORD dwEffect,float flSpeed);
	cMessage	cWarSpell::SpellAnim(cWarSpell* pcWarSpell,WORD wWarAnim,WORD wWarAnim2);
	cMessage	cWarSpell::SetPosition();
	cMessage	cWarSpell::SpellImpact(cWarSpell* pcWarSpell,DWORD dwEffect,float flSpeed);
	cMessage	cWarSpell::SpellVis(cWarSpell* pcWarSpell);
};

//************************************
//*	World objects - Monsters
//************************************
class cMonster : public cObject
{
	friend class cClient;
	friend class SimpleAI;

public:
//	cMonster		();
	/**
	 *	A constructor.
	 *
	 *	Called whenever a monster object is initialized.
	 */
	cMonster		( DWORD dwGUID, DWORD dwMonsterID, cLocation *pcLoc, char *szName, char *szDesc, cMonStats *pcmsStats, DWORD dwRespawn, DWORD dwDecay, DWORD dwChase, DWORD dwInfluence, DWORD dwExp_Value, DWORD dwHealth, DWORD dwStamina, DWORD dwMana );
	virtual ~cMonster		( ) {}	
	
	void		Assess					( cClient *pcAssesser );
	void		Action					( cClient* who );
	void		Attack					( cClient* who, float flDamageSlider, DWORD F7B0Sequence );
	void		SpellAttack				( cClient* who, cWarSpell *pcWarSpell, DWORD F7B0Sequence );
	static int	DeathAnimation			( LPVOID wp, LPVOID lp );
	cMessage	CreatePacket			( );
	cMessage	Animation				( WORD wAnim, float flPlaySpeed = 1.0f );
	cMessage	DoDamageMessage			( DWORD F7B0seq, std::string target, DWORD damagetype, double severity, DWORD amount );
	cMessage	RecieveDamageMessage	( DWORD F7B0seq, std::string giver, DWORD damagetype, double severity, DWORD amount, DWORD location );
	cMessage	DecrementHealth			( DWORD dwGUID, WORD amount, signed int &newhealth );
	cMessage	SetHealth				( DWORD dwNewHealth );
	cMessage	AdjustBar				( DWORD dwGUID, DWORD F7B0Sequence );
	cMessage	ChangeCombatMode		( bool fMode );
	cMessage	CombatAnimation			( DWORD dwTarget, WORD wAttackAnim );
	cMessage	TurnToTarget			( float flHeading, DWORD dwTargetGUID );
	cMessage	MoveToTarget			( cClient *pcWho );
	cMessage	MoveTarget				( cClient *pcWho );
	cMessage	ReturnToSpawn			( );
	void		ReSpawn					( cObject *pcObject );
	void        RemoveMonster			( cObject *pcObject );
	void		MonsterCorpse			( );
	cMessage	SetPosition				( );
	inline	DWORD	GetMonsterModelID	( ) { return m_dwMonsterModelID; }
	inline	WORD	GetTotalHealth		( ) { return m_dwCurrenthealth; }

	DWORD		m_dwMonsterModelID;
//	bool		m_fDeadOrAlive;
	WORD		m_wNumMovements;
	WORD		m_wNumAnimInteracts;
	WORD		m_wNumBubbleModes;
	WORD		m_wNumJumps;
	WORD		m_wNumPortals;
	WORD		m_wAnimCount;
	WORD		m_wNumOverrides;
	WORD		m_wNumLogins;
	// Character Stats
	DWORD		m_dwLevel;
	DWORD		m_dwCurrenthealth;
	DWORD		m_dwMaxHealth;
	DWORD		m_dwStr;
	DWORD		m_dwEnd;
	DWORD		m_dwQuick;
	DWORD		m_dwCoord;
	DWORD		m_dwFocus;
	DWORD		m_dwSelf;
	DWORD		m_dwCurrentStamina;
	DWORD		m_dwCurrentMana;
	DWORD		m_dwMaxStamina;
	DWORD		m_dwMaxMana;
	DWORD		m_dwSpecies;
	WORD		m_wCurAnim;
	WORD		m_wMeleeSequence;
	DWORD		m_dwF7B0Sequence;
	BYTE		m_bStatSequence;
	
	DWORD		m_dwUnknownCount;
	BYTE		m_bInitialAnimation[200];
	BYTE		m_bCombatMode;
	cLocation	m_TargetLocation;
	int			m_iPosUpdateCount;
	bool		m_fHasTarget;
	DWORD		m_dwTargetGUID;
};

#endif	// #ifndef __OBJECT_H