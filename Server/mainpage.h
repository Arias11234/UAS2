/**
@mainpage UAS2 Documentation


@section UAS2Intro Introduction

The Universal Asheron's Call Server project seeks to create an emulation server of the online game 
Asheron's Call while respecting the legal privileges retained by the game's developers.

UAS2 is a continuation of the UAS project, which can be found at http://sourceforge.net/projects/acemulator/.
The goal of UAS2 is to continue to develop the appropriate framework for an Asheron's Call server.
The collection of data that may be used to populate this framework is tasked to a sister project, UAS2db, 
which can also can found in the source.

The project source is located at http://sourceforge.net/projects/uas2.


@section UAS2Classes Class Overview

The entry point of the UAS2 application is WinMain in WinMain.cpp.

There are several top-level processess:
<ul>
	<li>cMasterServer:		Controls the loading and unloading of world data (threaded).
								<ul><li>
								The MasterServer thread is created as the game server is started.
								</li></ul>
		
	<li>cCharacterServer:	Receives and processes communication between the server and client when	the client 
							is NOT in-world.
								<ul>
								<li>Client login to and logout from the server.
								<li>Character creation and deletion.
								</ul>

	<li>cWorldServer:		Receives and processes communication between the server and client concerning avatar
							actions when the client is in-world.<br>
								<ul><li>
								Client character communication with the server.<br>
								(The Client/Server protocol used can be found on the 
								<a href = http://decal.insanity-inc.org/protocol/Documentation.aspx>
								Decal Documentation website</a>)
								</li></ul>

	<li>cMonsterServer:		Processes monster actions that are to be sent to the client. Located in cMonsterServer.cpp.
</ul>

There are several types of objects.  The base object class is cObject.  Most objects derive from this class.
<ul>
	<li>The avatar object is implemented in the cAvatar class.

	<li>Monster objects are implemented in the cMonster class.
		Monster AI is dictated by SimpleAI, while cMonsterServer (cMonsterServer.cpp) processes and sends the data.
	
	<li>World Objects (objects, generally static, loaded in the world) are implemented in their respective classes.
		All world objects are initialized using a constructor when the world is loaded.
		<ul><li>
		These classes include: cAltar, cChest, cCovenant, cDoor, cHooks, cHouse, cLifestone, 
		cMerchantSign, cPortal, cStorage, cWorldObject
		</li></ul>

	<li>Item Objects (non-static, usually inventoriable objects) are implemented in their respective classes.
		<ul><li>
		These classes include: cAmmo, cArmor, cBooks, cClothes, cFoci, cGems, cHealingCon, cHealingKits, 
		cJewelry, cLockpicks, cManaStones, cMisc, cPack, cPlants, cPyreals, cSalvage, cScrolls, cShield, 
		cSpellComps, cTradeNotes, cWands, cWeapon.
		</li></ul>
</ul>

The class cPortalDat handles the loading of information from the portal.dat file.<br>
The class cLandBlock handles the loading of information from the cell.dat file.<br>


@section UAS2DB UAS2DB Overview

UAS2DB is the collection of raw data to be used to populate the world of Asheron's Call.

The data is predominantely in the form of raw packet data, so as to capture and store all possible relevant information.

To be usable by UAS2, the relevant data must be extracted and converted into MySQL form and ultimately uploaded to the UAS2 database.
The UAS2 Capture Tool, or any similar tool, is intended to perform this function.
*/