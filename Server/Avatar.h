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
 *	@file Avatar.h
 */

#ifndef __AVATAR_H
#define __AVATAR_H

#include <algorithm>

#include "Object.h"

#define ACID_MODEL 102;
#define BLADE_MODEL 97;
#define FLAME_MODEL 85;
#define FORCE_MODEL 91;
#define FROST_MODEL 74;
#define LIGHTNING_MODEL 80;
#define SHOCK_MODEL 69;

class cAvatar : public cBiotic
{
	friend class cAllegiance;
	friend class cClient;
	friend class cDatabase;
	friend class cFellowship;
	friend class cPortalDat;
	friend class cCommandParser;
	friend class cMonsterServer;
	friend class cNPC;
	friend class cWorldManager;
	friend class cWeapon;
	friend class cWorldServer;

public:
	float m_flTargetHeading;
	cAvatar( cLocation *pcLoc = NULL ) 
		:	m_wCurAnim		( 5 ), 
			m_wMeleeSequence( 0 ), 
			m_wMoveCount	( 0 ),
			m_wPortalCount	( 1 ),
			m_pcCombatTarget( NULL ),
			cBiotic			( pcLoc ),
			m_bStatSequence	( 0 )
	{
		m_PreviousLocation.m_dwLandBlock = NULL;
		m_wNumLogins	= 0;
		m_wState		= 0;
		m_dwPingCount	= 0;
		m_wLifeStone	= 0;
		m_wMarketplace	= 0;
		m_wHouseRecall	= 0;
		m_bFlagCount	= 0;
		m_wPKlite		= 0;
	}

	cAvatar( DWORD guid, cLocation *pcLoc = NULL ) 
		:	m_wCurAnim		( 5 ), 
			m_wMoveCount	( 0 ), 
			m_wMeleeSequence( 0 ), 
			m_wPortalCount	( 0 ),
			m_pcCombatTarget( NULL ),
			cBiotic			( pcLoc ),
			m_bStatSequence	( 0 )
	{
		m_dwGUID		= guid; 
		m_wNumLogins	= 0;
		m_wState		= 0;
		m_dwPingCount	= 0;
		m_wLifeStone	= 0;
		m_wMarketplace	= 0;
		m_wHouseRecall	= 0;
		m_bFlagCount	= 0;
		m_wPKlite		= 0;
		m_wHouseID		= NULL;
	}

	~cAvatar( )
	{
		while ( !m_lstInventory.empty( ) )
		{
			SAFEDELETE( m_lstInventory.front( ) )
			m_lstInventory.pop_front( );
		}
	}

	cMessage		Portal					( DWORD dwLandblock, float flX, float flY, float flZ, float flA, float flW );

	/*agentsparrow - Spell routines*/
	static int		cAvatar::CastMain		( LPVOID wp, LPVOID lp );
	static int		cAvatar::WarAnimation1	( LPVOID wp, LPVOID lp );
	static int		cAvatar::WarAnimation2	( LPVOID wp, LPVOID lp );
	static int		cAvatar::WarAnimation3	( LPVOID wp, LPVOID lp );
	static void		cAvatar::CastWar		( LPVOID wp );
	static void		cAvatar::CastLife		( LPVOID wp );
	static void		cAvatar::CastCreature	( LPVOID wp );
	static void		cAvatar::CastItem		( LPVOID wp );
	DWORD			GetHealValue			( DWORD dwLevel );
	DWORD			GetStaminaValue			( DWORD dwLevel );
	float			GetRegenIncValue		( DWORD dwLevel );
	float			GetRegenDecValue		( DWORD dwLevel );
	float			GetProtValue			( DWORD dwLevel );
	float			GetVulnValue			( DWORD dwLevel );
	float			GetArmorValue			( DWORD dwLevel );
	float			GetTransferValue		( DWORD dwLevel );

	cMessage		LocationPacket			( );
	cMessage		SetPosition				( );
	void			SetOptions				( DWORD optionsMask );
	inline cMessage	Particle				( DWORD dwParticleID );
	cMessage		Animation				( WORD wAnim, float flPlaySpeed = 1.0f );
	cMessage		WarAnim					( WORD wAnim, float flPlaySpeed = 1.0f );
	cMessage		SoundEffect				( DWORD dwSound, float flPlaySpeed = 1.0f );
	cMessage		CreatePacket			( );
	cMessage		CreateLoginPacket		( DWORD F7B0seq );
	cMessage		SetPackContents			( DWORD F7B0seq );
	cMessage		AllegianceInfo			( DWORD F7B0seq );
	cMessage		HousingInfo				( DWORD F7B0seq );
	cMessage		LoginCharacter			( );
	void			LogoutCharacter			( );
	void			Assess					( cClient *pcAssesser );
	void			Action					( cClient *who )	 {}
	//Find the weapon the PC has equipped
	cMessage		ChangeCombatMode		( BOOL fMode, BYTE WieldType );
	cMessage		ChangeSpellMode			( BOOL fMode );
	cMessage		ChangeMissileMode		( BOOL fMode, WORD AmmoType);

	float			GetRange				( DWORD dwLandblock, float flX, float flY, float flZ, DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ );
	float			GetHeadingTarget		( DWORD dwLandblock, float flX, float flY, float flZ, DWORD dwTargetLandblock, float flTarX, float flTarY, float flTarZ );
	cMessage		RunToAnimation			( DWORD dwGUID, DWORD dwTarget );
	cMessage		CombatAnimation			( DWORD dwTarget );
	cMessage		TurnToTarget			( float flHeading, DWORD dwTargetGUID );

	cMessage		DoDamageMessage			( DWORD F7B0seq, std::string target, DWORD damagetype, double severity, DWORD amount );
	cMessage		RecieveDamageMessage	( DWORD F7B0seq, std::string giver, DWORD damagetype, double severity, DWORD amount, DWORD location );

	cMessage		DecrementHealth			( WORD amount, signed int &newhealth );
	cMessage		DecrementStamina		( WORD amount, signed int &newstamina );
	cMessage		DecrementMana			( WORD amount, signed int &newmana );
	cMessage		AddEnchant				( DWORD F7B0seq, DWORD dwSpellID, WORD wEnchantLayer, WORD wSpellFamily, DWORD dwDifficulty, double dDuration, DWORD dwCasterGUID, double dTime, DWORD dwFlags, DWORD dwKey, float flValue );
	cMessage		RemoveEnchant			( DWORD F7B0seq, DWORD dwSpellID, WORD wEnchantLayer );

	/* Statistic (burden, xp, rank, etc.) functions	*/
	void			UpdateHuntingExp		( DWORD amount );
	void			UpdateFellowshipExp		( DWORD amount );
	cMessage		UpdateUnassignedExp		( DWORD amount );
	cMessage		UpdateTotalExp			( DWORD amount );
	cMessage		UpdateSkillExp			( DWORD amount, BYTE skill );
	cMessage		UpdateLevel				( );
	cMessage		UpdateBurden			( DWORD amount, int action );
	cMessage		UpdateRank				( DWORD amount );
	cMessage		UpdatePyreals			( DWORD amount, int positive );
	cMessage		UpdateDeaths			( DWORD amount );
	cMessage		UpdateAge				( DWORD amount );

	//k109:  Implementation of removing experience.
	cMessage		DecrementUnassignedExp	( DWORD amount );

	/* Attribute functions	*/
	cMessage		UpdateStrength			( DWORD exp );
	cMessage		UpdateEndurance			( DWORD exp );
	cMessage		UpdateQuickness			( DWORD exp );
	cMessage		UpdateCoordination      ( DWORD exp );
	cMessage		UpdateFocus				( DWORD exp );
	cMessage		UpdateSelf				( DWORD exp );

	/* Vital functions */
	void			CalcVital				( WORD vitalID );
	cMessage		RaiseHealth				( DWORD exp );
	cMessage		RaiseStamina			( DWORD exp );
	cMessage		RaiseMana				( DWORD exp );

	cMessage		AdjustHealthBar			(DWORD dwGUID, DWORD F7B0Sequence );
	
	/* Skill functions */
	void			CalcSkill				( DWORD skillID );
	cMessage		UpdateSkill				( DWORD skillID, DWORD exp );

	cMessage		AttackCompleteMessage	( DWORD F7B0seq );
	cMessage		AttackBeginMessage		( DWORD F7B0seq, DWORD pcGUID );

	cMessage		SetHealth				( DWORD dwNewHealth );
	cMessage		SetStamina				( DWORD dwNewStamina );
	cMessage		SetMana					( DWORD dwNewMana );

	cMessage		UpdateHealth			( WORD amount, signed int &newvalue );
	cMessage		UpdateStamina			( WORD amount, signed int &newvalue );
	cMessage		UpdateMana				( WORD amount, signed int &newvalue );

	cMessage		HouseAbandon			( DWORD F7B0seq );
	void			HouseGuestAdd			( char strGuestName[50] );
	void			HouseGuestRemoveName	( char strGuestName[50] );
	void			HouseOpenClose			( DWORD IsOpen );
	void			HouseStorage			( char strGuestName[50], DWORD dwStorageSet );
	void			HouseBootName			( char strGuestName[50] );
	void			HouseStorageRemoveAll	( );
	cMessage		HouseGuestList			( DWORD F7B0seq );

	DWORD			CalculateDamage			( cObject *pcWeapon, float flPower, float flResistance );
	cMessage		WindupAnimation			( WORD wAnim1, float flPlaySpeed );
	cMessage		WarAnimation			( WORD wAnim1, float flPlaySpeed );
	cMessage		WarAnimation2			( float flPlaySpeed );
	void			SetLifestone			( cLocation myLifestone );
	cMessage		LifestoneRecall			( );
	cMessage		LSAnimate				( );
	cMessage		LSMessage				( );
	cMessage		MarketplaceRecall		( );
	cMessage		MPAnimate				( );
	cMessage		MPMessage				( );
	cMessage		HouseRecall				( );
	cMessage		HRAnimate				( );
	cMessage		HRMessage				( );
	cMessage		ConfirmPanelRequest		( DWORD F7B0seq, DWORD type, DWORD ConfirmSeq, std::string targetName );
	
	void			SetIsPK_DB				( );

	inline	DWORD	GetAllegianceID			( )	{ return m_dwAllegianceID; }
	inline	DWORD	GetFellowshipID			( )	{ return m_dwFellowshipID; }
	inline	WORD	GetGender				( )	{ return m_wGender; }
	inline	WORD	GetRace					( )	{ return m_wRace; }
	inline	WORD	GetRank					( )	{ return m_wRank; }

	//Cube:  We should have some functions that get the avatars current attributes values
	inline  DWORD	GetTotalStrength		( ) { return m_cStats.m_lpcAttributes[0].m_dwCurrent; }
	inline  DWORD	GetTotalEndurance		( ) { return m_cStats.m_lpcAttributes[1].m_dwCurrent; }
	inline  DWORD	GetTotalQuickness		( ) { return m_cStats.m_lpcAttributes[2].m_dwCurrent; }
	inline  DWORD	GetTotalCoordination	( ) { return m_cStats.m_lpcAttributes[3].m_dwCurrent; }
	inline  DWORD	GetTotalFocus			( ) { return m_cStats.m_lpcAttributes[4].m_dwCurrent; }
	inline  DWORD	GetTotalSelf			( ) { return m_cStats.m_lpcAttributes[5].m_dwCurrent; }

	inline	WORD	GetTotalHealth			( ) { return m_cStats.m_lpcVitals[0].m_dwCurrent; }
	inline  WORD    GetTotalStamina			( ) { return m_cStats.m_lpcVitals[1].m_dwCurrent; }
	inline  WORD    GetTotalMana			( ) { return m_cStats.m_lpcVitals[2].m_dwCurrent; }

	inline	WORD	GetTotalLoyalty			( )	{ return m_cStats.m_lpcSkills[36].m_dwTotal; }
	inline	WORD	GetTotalLeadership		( )	{ return m_cStats.m_lpcSkills[35].m_dwTotal; }

	inline	void	SetAllegianceID			( DWORD newAllegianceID )	{ m_dwAllegianceID = newAllegianceID; }
	inline	void	SetFellowshipID			( DWORD newFellowshipID )	{ m_dwFellowshipID = newFellowshipID; }

	//Functions for SQL updates
	void UpdateQuestCompletedTable			( DWORD dwAvatarGUID, DWORD quest_id );
	void SaveToDB							( );
	void UpdateAllegianceDB					( );
	void UpdateAvatarLocation				( );

	void AddItemToInvDB						( DWORD dwAvatarGUID, cObject *pcObject );
	void DeleteItemFromInvDB				( DWORD dwAvatarGUID, cObject *pcObject );
	void UpdateUnassignedXPDB				( cClient *who, DWORD exp );
	void UpdateTotalXPDB					( cClient *who, DWORD exp );
	void UpdateLevelDB						( cClient *who );

	//Change Model
	cMessage UpdateAvatarPalette			( );
	cMessage UpdateAvatarTexture			( );
	cMessage UpdateAvatarModel				( );

	//Allegiances
	DWORD	GetAllegianceFromDB				( );
	void	BreakAllegiance					( );
	void	SwearAllegiance					( std::string szTargetName, DWORD dwReply, DWORD dwSenderGUID );
	void	SwearAllegianceReply			( std::string szTargetName, DWORD dwReply );
	void	BreakAllegiance					( DWORD dwMemberGUID );
	void	BreakAllegianceReply			( std::string szMemberName );
	
	//Fellowships
	void	CreateFellowship				( DWORD F7B0Sequence, char strFellowName[50] );
	void	DisbandFellowship				( );
	void	OpenCloseFellowship				( DWORD dwOpen );
	void	FellowshipLeader				( DWORD dwNewLeaderGUID );
	void	FellowshipRecruitSend			( std::string szTargetName, DWORD dwReply, DWORD dwReceiptGUID );
	void	FellowshipRecruitRecv			( std::string szTargetName, DWORD dwReply, DWORD dwSenderGUID );
	void	FellowshipDismiss				( DWORD dwMemberGUID );

	//Total Experience
	inline DWORD AwardTotalExp(DWORD txpamount)
	{
		if (m_dwTotalXP < 4294967295)
			m_dwTotalXP = m_dwTotalXP + txpamount;
		else if (m_dwTotalXP > 4294967295)
			m_dwTotalXP = 4294967295;
		else if (m_dwTotalXP == 4294967295)
			m_dwTotalXP += 0;
		
		return m_dwTotalXP;
	}

	//k109:  Changed these functions to increase and decrease.

	//Unassigend Experience Increase
	inline DWORD AwardUnassignedXP(DWORD uxpamount)
	{
		if (m_dwUnassignedXP < 4294967295)
			m_dwUnassignedXP = m_dwUnassignedXP + uxpamount;
		else if (m_dwUnassignedXP > 4294967295)
			m_dwUnassignedXP = 4294967295;
		else if (m_dwUnassignedXP == 4294967295)
			m_dwUnassignedXP += 0;

		return m_dwUnassignedXP;
	}

	//Unassigend Experience Decrease
	inline int SpendUnassignedXP(int uxpamount)
	{
		m_dwUnassignedXP = m_dwUnassignedXP - uxpamount;
		return m_dwUnassignedXP;
	}

	//Add Burden
	inline int AddBurden(int amount)
	{
		m_dwBurden = m_dwBurden + amount;
		return m_dwBurden;
	}

	//Remove Burden
	inline int RemoveBurden(int amount)
	{
		m_dwBurden = m_dwBurden - amount;
		return m_dwBurden;
	}

	//Add Pyreals
	inline DWORD AddPyreals(DWORD amount)
	{
		m_dwPyreals = m_dwPyreals + amount;
		return m_dwPyreals;
	}

	//Remove Pyreals
	inline DWORD RemovePyreals(DWORD amount)
	{
		m_dwPyreals = m_dwPyreals - amount;
		return m_dwPyreals;
	}

	inline int EatStaminaFood(int amount)
	{
		int update = amount;
		return update;
	}
	
	/* Cubem0j0: Add damage reduction based on AL */
	inline int ALDamageReduction( DWORD alvalue )
	{
		float dam_reduce = (1/1+alvalue/60);
		return dam_reduce;
	}

	inline void SetCombatTarget( cObject *pcObject ) 
	{ 
		m_pcCombatTarget = pcObject; 
	}

	inline void AddInventory( cObject *pcObject )
	{
		m_lstInventory.push_back( pcObject );
		pcObject->m_fIsOwned = TRUE;
		pcObject->m_dwContainer = m_dwGUID;
	}

	inline void RemoveInventory( cObject *pcObject )
	{
		iterObject_lst itObject = std::find( m_lstInventory.begin( ), m_lstInventory.end( ), pcObject );

		if ( itObject != m_lstInventory.end( ) )
			m_lstInventory.erase( itObject );
		pcObject->m_fIsOwned = FALSE;
		pcObject->m_dwContainer = NULL;
	}

	inline cObject *FindInventory( DWORD dwGUID )
	{
		for ( iterObject_lst itObject = m_lstInventory.begin( ); itObject != m_lstInventory.end( ); ++itObject )
		{
			if ( dwGUID == (*itObject)->GetGUID( ) )
				return *itObject;
		}
		return NULL;
	}

	//k109:  AL :p
	inline DWORD SetArmorLevel (DWORD AL)
	{
		Armor_Level = AL;
		return AL;
	}

	inline DWORD GetLevel(DWORD totalxp)
	{
		unsigned long level = floor(pow((double)9 * totalxp + 7776,(float)1/5) - 6) + 1;
		return level;
	}

	inline void DeleteFromInventory( cObject *pcObject )
	{
		iterObject_lst itObject = std::find( m_lstInventory.begin( ), m_lstInventory.end( ), pcObject );

		if ( itObject != m_lstInventory.end( ) )
			m_lstInventory.erase( itObject );
	}

	void CreateChildren( cClient *pcClientDestination );

	WORD		m_wCurAnim;
	WORD		m_wMeleeSequence;
	WORD		m_wPortalCount;
	WORD		m_wMoveCount;
	BYTE		m_bStatSequence;
	WORD		m_wModelSequenceType;
	WORD		m_wModelSequence;
	WORD		m_wModelNum;
	float		m_flAScale;
	BYTE		m_bAccessLevel;
	WORD		m_wState;
	DWORD		m_dwPingCount;
	WORD		m_wLifeStone;
	WORD		m_wMarketplace;
	WORD		m_wHouseRecall;
	WORD		m_wHouseID;
	WORD		m_wPKlite;
	WORD		m_wTimedEvent;
	DWORD		m_dwNumFollowers;
	DWORD		m_dwNumDeaths;
	BYTE		m_bFlagCount;
		
	// Spell Variables
	WORD		m_wSpellCount;
	DWORD		m_dwSpellUnknown;
	cSpellBook	m_SpellBook[100];
	cSpellTab	m_SpellTabs[7];
	DWORD		m_dwInventoryCount;
	cInventory	m_Inventory[300];
	vInventory	m_vInventory[300];
	
	//Cube:  Array to hold inventory item GUIDs
	DWORD m_InvGUIDs[300];

	cLocation	m_PreviousLocation;
	cStats		m_cStats;
	BYTE		m_fIsPK;
	DWORD		m_CorpseTarget;
	DWORD		m_DmgTypeEquipped;

	//Fellowship stuff
	bool		inFellow;

	//Character Option Flag
	DWORD		m_dwOptions;
	
	std::list< cObject * >	m_lstInventory;
	std::string sAccount_Name;

	cWeapon *myWeapon;
	cShield *myShield;
	cAmmo	*myAmmo;

private:
	char		m_strCharacterName[32];

	DWORD		m_dwBurden;
	DWORD		m_dwPyreals;
	DWORD		m_dwTotalXP;
	DWORD		m_dwUnassignedXP;
	WORD		m_wRank;
	BYTE		m_bTotalSkillCredits;
	BYTE		m_bSkillCredits;
	DWORD		m_dwDeaths;
	DWORD		m_dwBirth;
	DWORD		m_dwNumLogins;
	
	//Cubem0j0: Add support for Avatar armor level.
	DWORD		Armor_Level;
	DWORD		m_dwTotal_exp_spent_strength;

	cObject		*m_pcCombatTarget;

	WORD		m_wRace;
	WORD		m_wGender;
	WORD		m_wClass;

	char		m_strRaceName[20];
	char		m_strSexName[20];
	char		m_strClassName[20];

	double		m_dblSkinShade;
	WORD		m_wHairColor;
	double		m_dblHairShade;
	WORD		m_wHairStyle;
	WORD		m_wEyeColor;

	WORD		m_wHead;
	WORD		m_wNose;
	WORD		m_wChin;

	WORD		m_wPaletteCode;

	//k109:  Added this for books.


	//k109:  Struct for palette:
	struct PaletteChange
	{
		WORD m_wNewPalette;
		BYTE m_ucOffset;
		BYTE m_ucLength;
	} pc;

	//k109: Struct for Texture Change
	#pragma pack( push, 1 )
	struct TextureChange
	{
		BYTE m_bModelIndex;
		WORD m_wOldTexture;
		WORD m_wNewTexture;
	} tc;
	#pragma pack(pop)

	//k109: Struct for Model Change
	#pragma pack( push, 1 )
	struct ModelChange
	{
		BYTE m_bModelIndex;
		WORD m_wNewModel;
	} mc;
	#pragma pack(pop)


	//k109: variables for holding Base Attributes
	int base_strength;
	int base_endurance;
	int base_coordination;
	int base_quickness;
	int base_focus;
	int base_self;

	//k109: variables for holding base Vitals
	int base_health;
	int base_stamina;
	int base_mana;

	//also need vars to hold current exp spent on Attributes
	DWORD strength_exp_cost;
	DWORD endurance_exp_cost;
	DWORD coordination_exp_cost;
	DWORD quickness_exp_cost;
	DWORD focus_exp_cost;
	DWORD self_exp_cost;

	//Vitals exp spent
	DWORD health_exp_cost;
	DWORD stamina_exp_cost;
	DWORD mana_exp_cost;

	/*
	//Base skills
	int base_axe;
	int base_bow;
	int base_crossbow;
	int base_dagger;
	int base_mace;
	int base_meleed;
	int base_missiled;
	int base_spear;
	int base_staff;
	int base_sword;
	int base_tweapons;
	int base_unarmed;
	int base_arcane;
	int base_magicd;
	int base_manac;
	int base_itemtink;
	int base_assess_person;
	int base_deception;
	int base_healing;
	int base_jump;
	int base_lockpick;
	int base_run;
	int base_assess_creature;
	int base_weapontink;
	int base_armortink;
	int base_mitink;
	int base_creature;
	int base_item;
	int base_life;
	int base_war;
	int base_leadership;
	int base_loyalty;
	int base_fletching;
	int base_alchemy;
	int base_cooking;
	int base_salvage;
	
	//Skills experience spent
	DWORD axe_exp_spent;
	DWORD bow_exp_spent;
	DWORD crossbow_exp_spent;
	DWORD dagger_exp_spent;
	DWORD mace_exp_spent;
	DWORD meleed_exp_spent;
	DWORD missiled_exp_spent;
	DWORD spear_exp_spent;
	DWORD staff_exp_spent;
	DWORD sword_exp_spent;
	DWORD tweapons_exp_spent;
	DWORD unarmed_exp_spent;
	DWORD arcane_exp_spent;
	DWORD magicd_exp_spent;
	DWORD manac_exp_spent;
	DWORD itemtink_exp_spent;
	DWORD assess_person_exp_spent;
	DWORD deception_exp_spent;
	DWORD healing_exp_spent;
	DWORD jump_exp_spent;
	DWORD lockpick_exp_spent;
	DWORD run_exp_spent;
	DWORD assess_creature_exp_spent;
	DWORD weapontink_exp_spent;
	DWORD armortink_exp_spent;
	DWORD mitink_exp_spent;
	DWORD creature_exp_spent;
	DWORD item_exp_spent;
	DWORD life_exp_spent;
	DWORD war_exp_spent;
	DWORD leadership_exp_spent;
	DWORD loyalty_exp_spent;
	DWORD fletching_exp_spent;
	DWORD alchemy_exp_spent;
	DWORD cooking_exp_spent;
	DWORD salvage_exp_spent;
	*/

	BYTE			m_bBasicPaletteChange;
	DWORD			m_wBasicPaletteVector;
	sPaletteChange	m_BasicVectorPal[255];

	BYTE			m_bBasicTextureChange;
	DWORD			m_wBasicTextureVector;
	sTextureChange	m_BasicVectorTex[255];
	
	BYTE			m_bBasicModelChange;
	DWORD			m_wBasicModelVector;
	sModelChange	m_BasicVectorMod[255];

	//Cube:  Change model variables
	BYTE			m_bPaletteChange;
	DWORD			m_wPaletteVector;
	sPaletteChange	m_vectorPal[255];

	BYTE			m_bTextureChange;
	DWORD			m_wTextureVector;
	sTextureChange	m_vectorTex[255];

	BYTE			m_bModelChange;
	DWORD			m_wModelVector;
	sModelChange	m_vectorMod[255];

	DWORD			m_dwAllegianceID;
	DWORD			m_dwFellowshipID;
};

//Inline Functions
//================

inline cMessage cAvatar::Particle( DWORD dwParticleID )
{
	cMessage cParticle;
	cParticle << 0xF755L << m_dwGUID << dwParticleID << 0x3F2B4E94L;
	
	return cParticle;
}

inline cMessage cAvatar::SoundEffect(DWORD dwSound, float flPlaySpeed )
{
	cMessage cParticle;
	cParticle << 0xF750L << m_dwGUID << dwSound << flPlaySpeed;
	
	return cParticle;
}

inline void cAvatar::SetLifestone( cLocation myLifestone )
{
	m_LSLoc.m_dwLandBlock	= myLifestone.m_dwLandBlock;
	m_LSLoc.m_flX			= myLifestone.m_flX;
	m_LSLoc.m_flY			= myLifestone.m_flY;
	m_LSLoc.m_flZ			= myLifestone.m_flZ;
	m_LSLoc.m_flA			= myLifestone.m_flA;
	m_LSLoc.m_flB			= myLifestone.m_flB;
	m_LSLoc.m_flC			= myLifestone.m_flC;
	m_LSLoc.m_flW			= myLifestone.m_flW;
}

#endif	// #ifndef __AVATAR_H