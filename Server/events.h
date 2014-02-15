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
 *	@file events.h
 */

#ifndef __EVENTS_H
#define __EVENTS_H

#define CHANGE_COMBAT_MODE		0x0053 // F7B1
#define CHARACTER_SPAWN			0x00A1 // F7B1
#define CAST_MAGIC				0x004A // F7B1
#define MOVEMENT_HIGH_PRIORITY	0xF753 // F7B1
#define MOVEMENT_LOW_PRIORITY	0xF61C // F7B1
#define JUMP					0xF61B // F7B1
#define TEXT_FROM_CLIENT		0x0015 // F7B1
#define ATTACK					0x0008 // F7B1
#define TELL_TO_KNOWN			0x0032 // F7B1
#define TELL_TO_UNKNOWN			0x005D // F7B1
#define USE						0x0036
#define ASSESS					0x00C8
#define INVENTORY_ADD_ADJUST	0x0019
#define INVENTORY_DROP			0x001B
#define INVENTORY_EQUIP			0x001A
#define GET_OBJECT_MANA			0x0263 // F7B1
#define RECIEVE_MELEE_DAMAGE	0x01B2
// @ Commands Recalls, PKLite (Type F7B1:xxxx)
#define LIFESTONE_RECALL		0x0063
#define MARKETPLACE_RECALL		0x028D
#define HOUSE_RECALL			0x0262
#define ALLEG_RECALL			0x0278 // Includes Mansion Recall
#define PKLITE					0x028F
#define SUICIDE					0x0279

// @House Commands (Type F7B1:xxxx)
#define HOUSE_ABANDON			0x021F
#define HOUSE_GUEST_ADD			0x0245
#define HOUSE_GUSET_REM_NAME	0x0246 // by name
#define HOUSE_GUEST_ALLEG		0x0267 // Add or Remove
#define HOUSE_GUEST_LIST		0x024D
#define HOUSE_GUEST_REM_ALL		0x025E // Remove all Guests from list
#define HOUSE_STORAGE			0x0249 // Add or Remove by Name
#define HOUSE_STORE_ALL			0x024C // Remove all from storage
#define HOUSE_BOOT_NAME			0x024A // Boot by Name
#define HOUSE_BOOT_ALL			0x025F // Boot Everyone
#define HOUSE_AVAIL				0x0270
#define HOUSE_OPEN_CLOSE		0x0247 // Open or Close
#define HOUSE_HOOKS				0x0266 // On or Off

// @allegiance Commands (Type F7B1:xxxx)
#define ALLEG_BOOT				0x0277 // Boot Member by Name
#define ALLEG_INFO				0x027B // Retrieve Info by Name

// @permit Commands (Type F7B1:xxxx)
#define PERMIT_ADD				0x0219 // Add player by Name
#define PERMIT_REM				0x021A // Remove User by Name

// @Consent Commands (Type F7B1:xxxx)
#define CONSENT_STATE			0x01A1 // Turns Consent On or Off
#define CONSENT_WHO				0x0217 // Who has given you permission to loot
#define CONSENT_REM				0x0218 // Removes Permission given by player
#define CONSENT_CLEAR			0x0216 // Clear Consent List

#endif