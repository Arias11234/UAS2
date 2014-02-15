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
 *	@file WorldServer.cpp
 *	Processes client messages according to their pre-defined types.
 */

//NOTES:  This file has been extensively modified from the original version.
//		  All of the known client (outbound) messages have been added to this code.

#include "Client.h"
#include "DatFile.h"
#include "MasterServer.h"
#include "RecvPacket.h"
#include "WorldManager.h"
#include "WorldServer.h"
#include "Job.h"
#include "cSpell.h"

// ** F7B1: xxxx **
#define ATTACK					0x0008
#define MISSILE_ATTACK			0x000A
#define TEXT_FROM_CLIENT		0x0015
#define INVENTORY_ADD_ADJUST	0x0019
#define INVENTORY_EQUIP			0x001A
#define INVENTORY_DROP			0x001B
#define SWEAR_ALLEGIANCE		0x001D
#define BREAK_ALLEGIANCE		0x001E
#define ALLEGIANCE_PANEL		0x001F
#define SEND_TELL_GUID			0x0032 // Not sure about this one.
#define TARGET_USE				0x0035 // Targeted Use (healing kits, etc); possibly used in trade skills as well.
#define USE						0x0036 // Normal Use
#define RAISE_VITAL				0x0044
#define RAISE_ATTRIBUTE			0x0045
#define RAISE_SKILL				0x0046
#define TRAIN_SKILL				0x0047
#define CAST_SPELL_NO_TARGET	0x0048
#define CAST_SPELL_TARGET		0x004A
#define CHANGE_COMBAT_MODE		0x0053
#define STACK_ITEMS				0x0054
#define SPLIT_ITEMS				0x0055
#define SQUELCH					0x0058
#define SQUELCH_ACCOUNT			0x0059
#define SEND_TELL_NAME			0x005D
#define VENDOR_BUY_ITEMS		0x005F // Buy Item(s) from Vendor
#define VENDOR_SELL_ITEMS		0x0060 // Sell Item(s) to Vendor	
#define LIFESTONE_RECALL		0x0063
#define CHARACTER_SPAWN			0x00A1
#define CREATE_FELLOWSHIP		0x00A2
#define DISBAND_FELLOWSHIP		0x00A3
#define DISMISS_FELLOW_MEMBER	0x00A4
#define RECRUIT_FELLOW_MEMBER   0x00A5
#define FELLOWSHIP_PANEL		0x00A6
#define WRITE_BOOK				0x00AB // Writing to a blank page in a book.
#define UNK						0x00AC
#define READ_BOOK				0x00AE
#define INSCRIBE				0x00BF // Inscribe Item
#define ASSESS					0x00C8 // Appraise and Assess
#define	GIVE_ITEM				0x00CD // Give Item (NPC)
#define REMOVE_PRIVS			0x00D3 // Strip Access Level?  May be useful.
#define TELE_TO_LB				0x00D6 // Admin command?
#define REPORT_ABUSE			0x0140
#define SEND_TELL_MASK			0x0147
#define CLOSE_CONTAINER_FORCED  0x0195 // Occurs automatically when you wander too far from a chest
#define MAKE_SHORTCUT			0x019C
#define REMOVE_SHORTCUT			0x019D
#define ADJUST_SETTINGS			0x01A1
#define SAVE_LIFESTONE_POS		0x01A2 // (@save)
#define DELETE_SPELL_SHORTCUT	0x01A8
#define STOP_ATTACK				0x01B7
#define REQUEST_HEALTH_UPDATE	0x01BF
#define RETRIEVE_AGE			0x01C2
#define RETRIEVE_BIRTH			0x01C4
#define EMOTE_TEXT				0x01DF
#define EMOTE_COMMAND_TEXT		0x01E1
#define ADD_SPELL_SHORTCUT		0x01E3
#define REMOVE_SPELL_SHORTCUT	0x01E4
#define TELEPORT_TO_PLAYER		0x01E6 //Admin command?  (@teleto)
#define PING_SERVER				0x01E9
#define BEGIN_TRADE				0x01F6
#define END_TRADE				0x01F7
#define ADD_ITEM_TO_TRADE		0x01F8
#define ACCEPT_TRADE			0x01FA
#define WITHDRAW_OFFER			0x01FB
#define CLEAR_TRADE_WINDOW		0x0204
#define CONSENT_CLEAR			0x0216
#define CONSENT_LIST			0x0217
#define CONSENT_REMOVE			0x0218
#define PERMIT_ADD				0x0219
#define PERMIT_REMOVE			0x021A
#define HOUSE_BUY				0x021C
#define HOUSE_MAINTAIN			0x0221
#define HOUSE_ABANDON			0x021F
#define HOUSE_BOOT_NAME			0x024A // Boot by Name
#define HOUSE_BOOT_ALL			0x025F // Boot Everyone
#define HOUSE_GUEST_ADD			0x0245
#define HOUSE_GUEST_REM_NAME	0x0246 // Remove Guest by Name
#define HOUSE_OPEN_CLOSE		0x0247 // Open or Close
#define HOUSE_STORAGE			0x0249 // Add or Remove by Name
#define HOUSE_BOOT_NAME			0x024A // Boot by Name
#define HOUSE_STORE_REM_ALL		0x024C // Remove All from Storage
#define HOUSE_GUEST_LIST		0x024D
#define SET_SPEAKER				0x0251
#define SHOW_SPEAKER			0x0252
#define CLEAR_SPEAKER			0x0253
#define SET_MOTD				0x0254
#define SHOW_MOTD				0x0255
#define CLEAR_MOTD				0x0256
#define HOUSE_GUEST_REM_ALL		0x025E // Remove all Guests from list
#define HOUSE_BOOT_ALL			0x025F // Boot Everyone
#define HOUSE_RECALL			0x0262
#define GET_OBJECT_MANA			0x0263 // Buggy...
#define HOUSE_HOOKS				0x0266 // On or Off
#define HOUSE_GUEST_ALLEG		0x0267 // Add or Remove
#define JOIN_CHESS_GAME			0x0269
#define LEAVE_CHESS_GAME		0x026A
#define MOVE_CHESS_PIECE		0x026B
#define OFFER_CHESS_DRAW		0x026E
#define HOUSE_AVAIL				0x0270
#define ANSWER_POPUP			0x0275 // Answers a Yes/No pop-up question.
#define ALLEG_BOOT				0x0277 // Boot Member by Name
#define ALLEG_RECALL			0x0278 // Includes Mansion Recall
#define SUICIDE					0x0279 // (@die)
#define ALLEG_INFO				0x027B // Retrieve Info by Name
#define MARKETPLACE_RECALL		0x028D
#define PKLITE					0x028F // (@pklite)
#define PROMOTE_FELLOW_LEADER	0x0290
#define OPEN_CLOSE_FELLOWSHIP	0x0291
#define JUMP					0xF61B
#define MOVEMENT_LOW_PRIORITY	0xF61C // Animations
#define MOVEMENT_HIGH_PRIORITY	0xF753 // Position Update

// ** F7B0: xxxx **

#define RECIEVE_MELEE_DAMAGE	0x01B2
#define SET_PACK_CONTENTS		0x0196 // Add items to pack/chest/etc

//Cubem0j0:  Found this one by accident, sent after you buy something:
//This is a sequenced message, after seq number comes 0x5F, GUID of the vendor,
//then seems to always be 0x00000001, then the amount purchased (DWORD), then
//lastly the GUID of the item.

// ** Skill defines ** /
#define AXE						0x00000001
#define BOW						0x00000002
#define CROSSBOW				0x00000003
#define DAGGER					0x00000004
#define MACE					0x00000005
#define MELEE_DEFENSE			0x00000006
#define MISSILE_DEFENSE			0x00000007
#define SPEAR					0x00000009
#define STAFF					0x0000000A
#define SWORD					0x0000000B
#define THROWN_WEAPONS			0x0000000C
#define UNARMED_COMBAT			0x0000000D
#define ARCANE_LORE				0x0000000E
#define MAGIC_DEFENSE			0x0000000F
#define MANA_CONVERSION			0x00000010
#define ITEM_TINKERING			0x00000012
#define ASSESS_PERSON			0x00000013
#define DECEPTION				0x00000014
#define HEALING					0x00000015
#define S_JUMP					0x00000016
#define LOCKPICK				0x00000017
#define RUN						0x00000018
#define ASSESS_CREATURE			0x0000001B
#define WEAPON_TINKERING		0x0000001C
#define ARMOR_TINKERING			0x0000001D
#define MAGIC_ITEM_TINKERING 	0x0000001E
#define CREATURE_ENCHANTMENT 	0x0000001F
#define ITEM_ENCHANTMENT		0x00000020
#define LIFE_MAGIC		 		0x00000021
#define WAR_MAGIC				0x00000022
#define LEADERSHIP				0x00000023
#define LOYALTY					0x00000024
#define FLETCHING				0x00000025
#define ALCHEMY					0x00000026
#define COOKING					0x00000027
#define SALVAGING				0x00000028

cWeapon *pcW;
cShield *pcS;
cAmmo	*pcMW;

//Initialize static members
//============================================
DWORD		cWorldServer::m_dwSendCRCSeed;
DWORD		cWorldServer::m_dwRecvCRCSeed;
WORD		cWorldServer::m_wLogicalID;
SOCKET		cWorldServer::m_Socket;
float		intRange;
DWORD		NPCID;
DWORD		CORPSEID;
//============================================

/**
 *	Processes World Server packets
 *
 *	This function is called when the client sends a general World Server message.
 *	These encompass the actions performed by a client's avatar.
 */
void cClient::ProcessPacket_WS( cRecvPacket* pcRecvPacket )
{
	m_dwLastRecvTime = timeGetTime( );
		
	switch ( pcRecvPacket->GetFlags( ) )
	{
		//Client request for the resending of packet(s)
		case 0x00000002:
		{
			#ifdef _DEBUG
			cMasterServer::ServerMessage( ColorGreen, this, "Incorrect server-to-client message detected." );
			#endif
				
			DWORD dwNumLost;
			WORD  wTime;
							
			wTime = pcRecvPacket->GetTime( );

			pcRecvPacket->CopyPayload( 0, &dwNumLost, 4 );	//the number of packets lost

			DWORD *dwLost = new DWORD[dwNumLost];	//DWORD array dwNumLost created to hold lost packets

			pcRecvPacket->CopyPayload( 4, dwLost, dwNumLost * 4 );	//the lost packets (of count dwNumLost)

			m_PacketPipe_WS.ProcessLostPackets( dwNumLost, dwLost, m_saSockAddr );	//process the lost packets
			SAFEDELETE_ARRAY( dwLost )
				
			return;
		}
		//Client confirmation that the previous packet sequence was received
		case 0x00000004:
		{ 
			DWORD dwLastGoodSeq;

			m_PacketPipe_WS.m_wTime = pcRecvPacket->GetTime( );
			pcRecvPacket->CopyPayload( 0, &dwLastGoodSeq, 4 );

			m_PacketPipe_WS.ClearSentPackets( dwLastGoodSeq );
			return ;
		}
		case 0x00100000:
		case 0x00200200:	//Cube:  This flag appears to be sent whenever movement updates occur (F753 / F61C)
		case 0x00000200:
		{
			m_PacketPipe_WS.m_dwRecvSequence	= pcRecvPacket->GetSequence( );
			m_PacketPipe_WS.m_wTime				= pcRecvPacket->GetTime( );

			if ( pcRecvPacket->GetFlags( ) == 0x00100000 )
			{
				m_PacketPipe_WS.ReturnPing( m_saSockAddr );
				m_pcAvatar->m_dwPingCount++;

				/* Process delayed or periodic actions */

				//Lifestone Recall
				if ((m_pcAvatar->m_dwPingCount > 7) && (m_pcAvatar->m_wLifeStone == 1))
					{
						AddPacket( WORLD_SERVER, m_pcAvatar->LifestoneRecall(), 3 );
						if(m_pcAvatar->m_LSLoc.m_dwLandBlock > 0x0000FFFF)
						{
							cWorldManager::MoveAvatar( this, m_pcAvatar->m_LSLoc, 1 );
						}
						m_pcAvatar->m_dwPingCount = 0;
						m_pcAvatar->m_wLifeStone = 0;
					}
				//Marketplace Recall
				else if((m_pcAvatar->m_dwPingCount > 7) && (m_pcAvatar->m_wMarketplace == 1))
					{
						AddPacket( WORLD_SERVER, m_pcAvatar->MarketplaceRecall(), 3 );
						cWorldManager::MoveAvatar( this,cMasterServer::m_MarketLoc, 1 );
						m_pcAvatar->m_dwPingCount = 0;
						m_pcAvatar->m_wMarketplace = 0;
					}
				//House Recall
				else if((m_pcAvatar->m_dwPingCount > 7) && (m_pcAvatar->m_wHouseRecall == 1))
					{
						AddPacket( WORLD_SERVER, m_pcAvatar->HouseRecall(), 3 );
						if(m_pcAvatar->m_HRLoc.m_dwLandBlock > 0x0000FFFF)
						{
							cWorldManager::MoveAvatar( this, m_pcAvatar->m_HRLoc, 1 );
						}
						m_pcAvatar->m_dwPingCount = 0;
						m_pcAvatar->m_wHouseRecall = 0;
					}
				//PK Lite
				else if ((m_pcAvatar->m_dwPingCount > 7) && (m_pcAvatar->m_wPKlite == 1))
				{
					cMasterServer::PKLite( this, true );
					m_pcAvatar->m_dwPingCount = 0;
					m_pcAvatar->m_wPKlite = 0;
				}

				//Health regeneration
				if( m_pcAvatar->m_cStats.m_lpcVitals[0].m_dwCurrent > m_pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent )
				{
					AddPacket( WORLD_SERVER, m_pcAvatar->SetHealth(m_pcAvatar->m_cStats.m_lpcVitals[0].m_lTrueCurrent + 1), 4 );
				}
				//Stamina regeneration
				if( m_pcAvatar->m_cStats.m_lpcVitals[1].m_dwCurrent > m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent )
				{
					AddPacket( WORLD_SERVER, m_pcAvatar->SetStamina(m_pcAvatar->m_cStats.m_lpcVitals[1].m_lTrueCurrent + 1), 4 );
				}
				//Mana regeneration
				if( m_pcAvatar->m_cStats.m_lpcVitals[2].m_dwCurrent > m_pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent )
				{
					AddPacket( WORLD_SERVER, m_pcAvatar->SetMana(m_pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent + 1), 4 );
				}

				//Update database record of avatar location
				m_pcAvatar->UpdateAvatarLocation();

				return;
			}

			break;	//case 0x00100000, case 0x00200200, and case 0x00000200
		}
	}

	switch ( m_bWorldState )
	{
		case 0:	//First login
		{
			if ( pcRecvPacket->GetFlags( ) == 0x00080000 )
			{
				m_bWorldState = 2;

				char szIP[32];
				cMasterServer::FormatIP_Port( m_saSockAddr, szIP );
				SendPacket100( );
			}
			break;
		}
		case 1:	//Subsequent logins
		{
			if ( pcRecvPacket->GetFlags( ) == 0x00080000 )
			{
				m_bWorldState = 2;
				
				m_PacketPipe_WS.Reset( );
				SendPacket100( );
			}
			break;
		}
		case 2:
		{
			if ( pcRecvPacket->GetFlags( ) == 0x100 ) //100 message acknowledged
			{
 				m_bWorldState = 3;
				SendPacket400( );
			}
			break;
		}
		case 3:
		{
			if ( pcRecvPacket->GetFlags( ) == 0x400 ) //400 message acknowledged
			{
				m_bWorldState = 4;
				cMasterServer::CreateInventory( this );	//Send create packets for inventory contents
				cMasterServer::SendLoginData( this );	//Send login data (uses CreateInventory information)
				m_pcAvatar->m_wLifeStone = 0;

				//Display console text information for a World Server connection
				char szIP[32];
				cMasterServer::FormatIP_Port( m_saSockAddr, szIP );
				UpdateConsole(	" World Server connection:\r\n"
								"      User: %s\r\n"
								"      Avatar: %s\r\n"
								"      IP: %s\r\n", this->m_szAccountName, m_pcAvatar->m_strName.c_str(), szIP);
			}
			break;
		}
		case 4:	//General World Packet
		{
			cFragmentHeader FH;
			DWORD MType1, MType2, MType3, MType4;
			BYTE *pbData;
			MType1 = 0x0L;
			MType2 = 0x0L;
			MType3 = 0x0L;  //Cubem0j0: for part 1 of F7B1:0045, 0046, 0047 messages
			MType4 = 0x0L;  //Cubem0j0: for part 2 of F7B1:0045, 0046, 0047 messages

			if ( pcRecvPacket->GetFlags( ) & 0x00000200 )
			{
				if ( pcRecvPacket->GetFlags( ) == 0x00200200 ) 
				{
					pbData = (BYTE *)&(*pcRecvPacket)[sizeof( cTransportHeader ) + 6];
				}
				else if ( pcRecvPacket->GetFlags( ) == 0x00000200 || pcRecvPacket->GetFlags( ) == 0x00000201 )
				{
					pbData = pcRecvPacket->GetPayloadFront( );
					
				}

				while ( pbData < pcRecvPacket->GetPayloadBack( ) )
				{
					CopyMemory( &FH, pbData, sizeof( cFragmentHeader ) );
					pbData += sizeof( cFragmentHeader );		
					CopyMemory( &MType1, pbData, 4 );
					CopyMemory( &MType2, &pbData[8], 4 );
					CopyMemory( &MType3, &pbData[12], 4 );
					CopyMemory( &MType4, &pbData[16], 4);

					switch ( MType1 )
					{
						case 0xF653:
						{	//Exit world
							m_bWorldState = 5;
							break;
						}
						case 0xF7A9:
						{	//Client needs landblock
							cMasterServer::ServerMessage( ColorGreen,this, "!telemap to another location or try typing /render radius 5." );
							#ifdef _DEBUG
								UpdateConsole( " Landblock data needed. \r\n" );
							#endif // _DEBUG
							//ServerMessage(UserParsing, "* You don't have at least one of the landblocks in your new area.  !telemap to another location or try typing /render radius 5.", COLOR_RED);
							break;
						}
						case 0xF6EA:
						{
							//Object no longer exsists
							break;
						}
						case 0xF7B1:
						{
							switch ( MType2 )
							{
								/* 
								Attack Types
									Melee:		ATTACK:				0x0008
									Missile:	MISSILE_ATTACK:		0x000A
									Magic:		CAST_SPELL_TARGET: 	0x004A
								*/

								//The client performs a melee attack
								case ATTACK:		//0x0008
								{
									//Attack target GUID
									DWORD dwTargetGUID =  *( DWORD * )&pbData[12];

									//Search the client list by GUID for the attack target
									cClient* pcTargetObj = cClient::FindClient( dwTargetGUID );
									
									//If no client is found (in which case the target is NOT another player)
									if( !pcTargetObj )
									{
										cObject* pcTargetObj = cWorldManager::FindObject( dwTargetGUID );
										
										intRange = m_pcAvatar->GetRange( m_pcAvatar->m_Location.m_dwLandBlock,m_pcAvatar->m_Location.m_flX,m_pcAvatar->m_Location.m_flY,m_pcAvatar->m_Location.m_flZ, pcTargetObj->m_Location.m_dwLandBlock, pcTargetObj->m_Location.m_flX, pcTargetObj->m_Location.m_flY, pcTargetObj->m_Location.m_flZ );
										
										//If beyond attack range
										if(intRange > pcTargetObj->m_fApproachDistance)
										{
											//cMessage Anim = m_pcAvatar->RunToAnimation( m_pcAvatar->m_dwGUID, pcTargetObj->GetGUID());
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->RunToAnimation( m_pcAvatar->m_dwGUID, pcTargetObj->GetGUID()), 3 );
											cMasterServer::ServerMessage(ColorGreen,this,"%s is too far away to attack.",pcTargetObj->m_strName.c_str());
											break;
										}
										//If not beyond attack range
										else if (intRange <= pcTargetObj->m_fApproachDistance)
										{
										
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->RunToAnimation( m_pcAvatar->m_dwGUID, pcTargetObj->GetGUID()), 2 );
											
											cMessage Anim = m_pcAvatar->RunToAnimation( m_pcAvatar->m_dwGUID, pcTargetObj->GetGUID());
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 2 );
											
											/*
											DWORD dwDamageType = 0x04;
											
											if (m_pcAvatar->myWeapon)
												dwDamageType = m_pcAvatar->myWeapon->m_dwDamageType;
											*/
											float flDamageSlider = *( float * )&pbData[20];
											
											if(( pcTargetObj->m_dwObjectFlags1 & 0x00000010 )&&(pcTargetObj->m_fDeadOrAlive == true)) 
											{
												pcTargetObj->Attack( this , flDamageSlider, ++m_dwF7B0Sequence);
												SimpleAI::SetUnderAttack( dwTargetGUID, m_pcAvatar->GetGUID() );
											}
											AddPacket( WORLD_SERVER, m_pcAvatar->AttackCompleteMessage( ++m_dwF7B0Sequence ), 4 );
										}
										
										break;
									}

									//If a client is found (in which case the target is another player)

									#ifdef _DEBUG
										char szMessage[155];
										sprintf( szMessage, " PK: %f %f",pcTargetObj->m_pcAvatar->m_fIsPK,m_pcAvatar->m_fIsPK );
										UpdateConsole((char *)szMessage); 
									#endif

									if( ( pcTargetObj->m_pcAvatar->m_fIsPK == 0) && ( m_pcAvatar->m_fIsPK == 0 ) )	
										break;
									
									//Find the distance to the target
									intRange = m_pcAvatar->GetRange( m_pcAvatar->m_Location.m_dwLandBlock,m_pcAvatar->m_Location.m_flX,m_pcAvatar->m_Location.m_flY,m_pcAvatar->m_Location.m_flZ, pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );
									
									//If the target is out of range
									if (intRange > 10)
										{
											cMasterServer::ServerMessage(ColorGreen,this,"The target is out of range. :(");

											//cMessage cmCompleteAttackMessage = m_pcAvatar->AttackCompleteMessage( ++m_dwF7B0Sequence );
											AddPacket( WORLD_SERVER, m_pcAvatar->AttackCompleteMessage( ++m_dwF7B0Sequence ), 4 );
											break;
										}						
									else if (intRange > 3)
										{

											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->Animation( 0x07L, 2.0f ), 3 );

											AddPacket( WORLD_SERVER, m_pcAvatar->AttackCompleteMessage( ++m_dwF7B0Sequence ), 4 );
											break;
										}
									else if (intRange > 1)
										{
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->RunToAnimation( m_pcAvatar->m_dwGUID, pcTargetObj->m_pcAvatar->GetGUID()), 2 );
											break;
										}
									DWORD dwDamageType = 0x04;

									if (m_pcAvatar->myWeapon->m_fEquipped == 1)
										dwDamageType = m_pcAvatar->myWeapon->m_dwDamageType;
									
									float flDamageSlider = *( float * )&pbData[20];

									//Take a swing
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->CombatAnimation( pcTargetObj->m_pcAvatar->GetGUID() ), 3 );

									WORD wTotalOpponentHealth = pcTargetObj->m_pcAvatar->GetTotalHealth();
									//Now perform a very basic combat calculation..no where near accurate.
									
									DWORD dwDamage = m_pcAvatar->CalculateDamage( NULL, flDamageSlider, 0.0f );
									double dSeverity = ( double ) wTotalOpponentHealth / dwDamage;
									
									int iNewHealth;

									BOOL fKilledPlayer = FALSE;
						
									//Following variable(s) only used if there is a kill
									cMessage cmAnim;
									cMessage cmBeginMessage = m_pcAvatar->AttackBeginMessage( ++m_dwF7B0Sequence, m_pcAvatar->GetGUID( ) );
									cMessage cmHealthLoss = pcTargetObj->m_pcAvatar->DecrementHealth( dwDamage, iNewHealth );

									//If the attacked player is killed
									if( iNewHealth <= 0 ) 
									{
										fKilledPlayer = TRUE;
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0 ), 3 );
										cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->Animation( 0x11L, 2.0f ), 3 );		
																
										cMasterServer::Corpse( pcTargetObj );

										cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0), 3 );
										cWorldManager::TeleportAvatar( pcTargetObj, cMasterServer::m_StartingLoc );
										
										char szDeathNotice[200];
										wsprintf( szDeathNotice, "%s has killed %s", m_pcAvatar->m_strName.c_str(), pcTargetObj->m_pcAvatar->m_strName.c_str( ) );
										
										pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->SetHealth( pcTargetObj->m_pcAvatar->GetTotalHealth( ) ), 4 );
										
									}
									//If the attacked player is not killed
									else
									{

										AddPacket(WORLD_SERVER,m_pcAvatar->CombatAnimation( dwTargetGUID ),4);

										pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->RecieveDamageMessage( ++pcTargetObj->m_dwF7B0Sequence, m_pcAvatar->m_strName, dwDamageType, dSeverity, dwDamage, 0x7 ), 4 );
	 									
										AddPacket( WORLD_SERVER, m_pcAvatar->DoDamageMessage( ++m_dwF7B0Sequence, pcTargetObj->m_pcAvatar->m_strName, dwDamageType, dSeverity, dwDamage), 4 );
									}

									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->CombatAnimation( dwTargetGUID ), 3 );
									AddPacket( WORLD_SERVER, m_pcAvatar->AttackBeginMessage( ++m_dwF7B0Sequence, m_pcAvatar->GetGUID( ) ), 4 );
									
									pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->DecrementHealth( dwDamage, iNewHealth ), 4 );

									AddPacket( WORLD_SERVER, m_pcAvatar->AttackCompleteMessage( ++m_dwF7B0Sequence ), 4 );
									break;
								}


								//The client performs a missile attack
								case MISSILE_ATTACK:		//0x000A
								{
									break;
								}
								
								//Local chat
								case TEXT_FROM_CLIENT: 		//0x0015
								{
									WORD wTextLength = FH.m_wFragmentLength - 14 - sizeof( cFragmentHeader );

									if ( wTextLength > 0 )
									{
										char *szMessageText = new char[wTextLength+1];
										CopyMemory( szMessageText, &pbData[14], wTextLength );
										szMessageText[wTextLength] = '\0';

										cMasterServer::ParseCommand( szMessageText, wTextLength, this );

										SAFEDELETE_ARRAY( szMessageText )
									}
									break;	//case 0x0015
								}
								
								//Picking up or unequipping an object
								case INVENTORY_ADD_ADJUST: 	//0x0019
								{
									DWORD dwObjectGUID = *( DWORD * )&pbData[12];
									if ( !dwObjectGUID )
										break;

									DWORD dwDestination = *( DWORD * )&pbData[20];
									
									//Search for the object in the avatar's inventory
									cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );

									//If the object was found in the avatar's inventory
									if( pcObj )	//Move object into inventory
									{
										if( pcObj->m_fEquipped != 0 )	//If the object was equipped
										{

											//Insert Inventory Item
											cMessage cmInsertInventoryItem;
											cmInsertInventoryItem	<< 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x22L << dwObjectGUID << m_pcAvatar->GetGUID( ) << dwDestination << 0L;
											AddPacket( WORLD_SERVER, cmInsertInventoryItem, 4 );

											unsigned char ucSound[16] = {
												0x50, 0xF7, 0x00, 0x00, 0xD6, 0xFD, 0x09, 0x50, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
											};
											CopyMemory( &ucSound[4], &m_pcAvatar->m_dwGUID, 4 );
											cMessage cmSound;
											cmSound.CannedData( ucSound, 16 );
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmSound, 3 );

											cMessage cWrChange;
											//Change Model
											WORD wModelChangeType = m_pcAvatar->m_wNumLogins;

											pcObj->m_fEquipped = 0;

											char	szCommand[255];
											RETCODE	retcode;
											sprintf( szCommand, "UPDATE items_instance_inventory SET Equipped = %d WHERE GUID = %lu AND OwnerGUID = %lu;",pcObj->m_fEquipped,dwObjectGUID,m_pcAvatar->GetGUID() );
											retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
											retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
											retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );								CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

											int paletteChange = m_pcAvatar->m_bBasicPaletteChange;
											int textureChange = m_pcAvatar->m_bBasicTextureChange;
											//Loop through the palettes and textures of previously equipped items
											for ( iterObject_lst itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
														<< DWORD(m_pcAvatar->m_dwGUID)
														<< BYTE(0x11)  // 11 Vector Palettes
														<< BYTE(paletteChange)
														<< BYTE(textureChange)
														<< BYTE(m_pcAvatar->m_bBasicModelChange);

											cWrChange << WORD(0x007E);
		
											//Loop through the avatar's default palettes
											if ( m_pcAvatar->m_bBasicPaletteChange != 0)
											{
												for (int i = 0; i < m_pcAvatar->m_bBasicPaletteChange; i++)
											{
													cWrChange.pasteData((UCHAR*)&m_pcAvatar->m_BasicVectorPal[i],sizeof(m_pcAvatar->m_BasicVectorPal[i]));
													//cWrChange.pasteData((UCHAR*)&m_pcAvatar->pc,sizeof(m_pcAvatar->pc));
												}
											}
											//Loop through the palettes of previously equipped items
											for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
											if ( m_pcAvatar->m_bBasicTextureChange != 0)
											{
												for (int i = 0; i < m_pcAvatar->m_bBasicTextureChange; i++)
												{
													cWrChange.pasteData((UCHAR*)&m_pcAvatar->m_BasicVectorTex[i],sizeof(m_pcAvatar->m_BasicVectorTex[i]));
													//cWrChange.pasteData((UCHAR*)&m_pcAvatar->pc,sizeof(m_pcAvatar->pc));
												}
											}
											//Loop through the textures of previously equipped items
											for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
											if ( m_pcAvatar->m_bBasicModelChange != 0) 
											{
												//Loop through the avatar's default models
												//Do not include avatar models superceded by item models
												for (int i = 0; i < m_pcAvatar->m_bBasicModelChange; i++)
												{
													modelIsCovered = false;

													//Loop through the models of previously equipped items
													for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
																	if(m_pcAvatar->m_BasicVectorMod[i].m_bModelIndex == pcItemInv->m_WearVectorMod[j].m_bModelIndex)
																	{
																		modelIsCovered = true;
																	}
																}
															}
														}
													}

													if (!modelIsCovered)
														cWrChange.pasteData((UCHAR*)&m_pcAvatar->m_BasicVectorMod[i],sizeof(m_pcAvatar->m_BasicVectorMod[i]));
												}
											}
											//Loop through the models of currently equipped items
											//If two items cover the same area, the one with a higher coverage value supercedes
											for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
											{
												if ( ( *itObject )->m_fEquipped == 2 )
												{
													cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
													if ( pcItemInv->m_bWearModelChange != 0)
													{
														for (int i = 0; i < pcItemInv->m_bWearModelChange; i++)
														{
															modelIsCovered = false;
															for ( iterObject_lst itObject2 = m_pcAvatar->m_lstInventory.begin( ); itObject2 != m_pcAvatar->m_lstInventory.end( ); ++itObject2 )
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
											cWrChange	<< m_pcAvatar->m_wNumLogins;
											cWrChange	<< ++m_pcAvatar->m_wModelSequence;
											cWorldManager::SendToAllInFocus(m_pcAvatar->m_Location,cWrChange,3);

											//Move Object to Inventory
											cMessage cmMoveObjectToInventory1;
											cmMoveObjectToInventory1 << 0xF74AL << dwObjectGUID << pcObj->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmMoveObjectToInventory1, 3 );
											
											
											break;
										}
										else		//If the object was not equipped
										{
											cMessage cmMoveInventory;
											cmMoveInventory	<< 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x22L << dwObjectGUID << m_pcAvatar->GetGUID( ) << dwDestination << 0L;
											AddPacket( WORLD_SERVER, cmMoveInventory, 4 );

											cMessage cmSetContainer;
											cmSetContainer << 0x022DL << ( pcObj->m_bInventorySequence += 2 ) << dwObjectGUID << 2L << m_pcAvatar->GetGUID( );
											AddPacket( WORLD_SERVER, cmSetContainer, 4 );

											break;
										}
									}
									//If the object is found in the world (non-inventory)
									else if( ( pcObj = cWorldManager::FindObject( dwObjectGUID ) ) )
									{
										//Bend Down Animation
										unsigned char ucAnimBendDown[28] = {
											0x4C, 0xF7, 0x00, 0x00, 0xFF, 0xF2, 0x09, 0x50, 0x3C, 0x00, 0x0A, 0x00, 0x04, 0x00, 0x00, 0x00,   //(L÷_____P________) - 0000
											0x00, 0x00, 0x3D, 0x00, 0x02, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,                           //(__=_________)     - 0010
										};
										CopyMemory( &ucAnimBendDown[4], &m_pcAvatar->m_dwGUID, 4 );
										CopyMemory( &ucAnimBendDown[8], &m_pcAvatar->m_wNumLogins, 2 );
										CopyMemory( &ucAnimBendDown[10], &( ++m_pcAvatar->m_wCurAnim ), 2 );
										CopyMemory( &ucAnimBendDown[12], &( ++m_pcAvatar->m_wMeleeSequence ), 2 );
										cMessage cmBendDownAnim;
										cmBendDownAnim.CannedData( ucAnimBendDown, 28 );
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmBendDownAnim, 3 );
										
										//Insert Inventory Item
										cMessage cmInsertInventoryItem;
										cmInsertInventoryItem << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x22L << dwObjectGUID << m_pcAvatar->GetGUID( ) << dwDestination << 0L;
										AddPacket( WORLD_SERVER, cmInsertInventoryItem, 4 );

										//Sound
										unsigned char ucSound[16] = {
											0x50, 0xF7, 0x00, 0x00, 0xFF, 0xF2, 0x09, 0x50, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,   //(P÷_____Pz______?) - 0000
										};
										CopyMemory( &ucSound[4], &m_pcAvatar->m_dwGUID, 4 );
										cMessage cmSound;
										cmSound.CannedData( ucSound, 16 );
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmSound, 3 );
										
										//Standup Animation
										unsigned char ucAnimStand[24] = {
											0x4C, 0xF7, 0x00, 0x00, 0xFF, 0xF2, 0x09, 0x50, 0x3C, 0x00, 0x0B, 0x00, 0x05, 0x00, 0x00, 0x00,   //(L÷_____P________) - 0000
											0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00,                                                   //(__=_____)         - 0010
										};
										CopyMemory( &ucAnimStand[4], &m_pcAvatar->m_dwGUID, 4 );
										CopyMemory( &ucAnimStand[8], &m_pcAvatar->m_wNumLogins, 2 );
										CopyMemory( &ucAnimStand[10], &( ++m_pcAvatar->m_wCurAnim ), 2 );
										CopyMemory( &ucAnimStand[12], &( ++m_pcAvatar->m_wMeleeSequence ), 2 );
										cMessage cmAnimStand;
										cmAnimStand.CannedData( ucAnimStand, 24 );
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmAnimStand, 3 );
									
										//Move the object to the avatar's inventory
										cMessage cmMoveObjectToInventory;
										cmMoveObjectToInventory << 0xF74AL << dwObjectGUID << pcObj->m_wNumLogins << ++pcObj->m_wPositionSequence;
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmMoveObjectToInventory, 3 );
										
										//Save the object to the avatar's inventory in the database
										cDatabase::AddToInventoryDB(m_pcAvatar->GetGUID(), pcObj);
										
										//Cubem0j0: see if this even fires...
										#ifdef _DEBUG
										cMasterServer::ServerMessage(ColorYellow,this,"RemoveObject Fired.");
										#endif
										
										cWorldManager::RemoveObject( pcObj, false, false );
										m_pcAvatar->AddInventory( pcObj );
										break;
									}

									//Check to see if the item is on a monster corpse
									cCorpse *monCorpse = cWorldManager::FindCorpse(m_pcAvatar->m_CorpseTarget);
									cObject *monObj = monCorpse->FindInventory(MType3);

									//If the object is found on the monster corpse
									if( monObj ) 
									{
										#ifdef _DEBUG
										cMasterServer::ServerMessage( ColorYellow,this,"Corpse id: %d",monCorpse->GetGUID());
										#endif

										cMessage cmPickUpItem;
										cmPickUpItem << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0022L << MType3
										<< m_pcAvatar->GetGUID() << 0L << 0L;
										AddPacket(WORLD_SERVER,cmPickUpItem, 4);

										#ifdef _DEBUG
										cMasterServer::ServerMessage( ColorYellow,this,"Object id: %d",monObj->GetGUID());
										#endif

										monCorpse->RemoveInventory(monObj);
										m_pcAvatar->AddInventory(monObj);
										
										break;
									}

									break;	//case 0x0019
								}
								
								//Equipping an object
								case INVENTORY_EQUIP:		//0x001A
								{
									DWORD dwObjectGUID = MType3;
									if ( !dwObjectGUID )
										break;	//case 0x001A

									DWORD dwCoverage = MType4;

									//Cubem0j0:  Find the model

									//Outside of Inventory
									//cObject *wObj = cWorldManager::FindObject(dwObjectGUID);
									//cItemModels *woItem = cItemModels::FindModel(wObj->GetItemModelID());

									//Search inside the avatar's inventory
									cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );
									
									//If the object was found in the avatar's inventory
									if ( pcObj )
									{
										cItemModels *pcItem = cItemModels::FindModel(pcObj->GetItemModelID());
										pcObj->m_fEquipped = 0;
										DWORD dwSequenceB = 0;
										BYTE  bSequenceA = 0;
										bSequenceA = ++pcObj->m_bInventorySequence;
										dwSequenceB = bSequenceA + 1;

										cMessage cmEquip; // Message 1 Wearing Clothing
										cmEquip << 0xF7B0L << m_pcAvatar->m_dwGUID << ++m_dwF7B0Sequence << 0x23L << dwObjectGUID << dwCoverage;
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmEquip, 4 );

										//If the object is not a weapon
										if (dwCoverage < 0x00100000)
										{

											cMessage cWrChange;

											WORD wModelChangeType = m_pcAvatar->m_wNumLogins;

											if (pcItem->m_PortalLinker != 0)
											{
												if (m_pcAvatar->m_wGender == 0)
													cPortalDat::LoadItemModel(pcObj, 0x0200004E);
												else
													cPortalDat::LoadItemModel(pcObj, 0x02000001);
											}

											int paletteChange = m_pcAvatar->m_bBasicPaletteChange + pcObj->m_bWearPaletteChange;
											int textureChange = m_pcAvatar->m_bBasicTextureChange + pcItem->m_bWearTextureChange;

											//Loop through the palettes and textures of previously equipped items
											for ( iterObject_lst itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
														<< DWORD(m_pcAvatar->m_dwGUID)
														<< BYTE(0x11)  // 11 Vector Palettes
														<< BYTE(paletteChange)
														<< BYTE(textureChange)
														<< BYTE(m_pcAvatar->m_bBasicModelChange);

											//cWrChange << m_pcAvatar->m_wPaletteCode;
											cWrChange << WORD(0x007E);

											//Loop through the avatar's default palettes
											if ( m_pcAvatar->m_bBasicPaletteChange != 0)
											{
												for (int i = 0; i < m_pcAvatar->m_bBasicPaletteChange; i++)
											{
													cWrChange.pasteData((UCHAR*)&m_pcAvatar->m_BasicVectorPal[i],sizeof(m_pcAvatar->m_BasicVectorPal[i]));
												}
											}
											//Loop through the palettes of previously equipped items
											for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
											{
												if ( ( *itObject )->m_fEquipped == 2 )
												{
													cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
													if ( ( *itObject )->m_bWearPaletteChange != 0)
													{
														for (int i = 0; i < ( *itObject )->m_bWearPaletteChange; i++)
														{
															cWrChange.pasteData((UCHAR*)&( *itObject )->m_WearVectorPal[i],sizeof(pcItemInv->m_WearVectorPal[i]));
														}
													}
												}
											}
											//Loop through the palettes of the newly equipped item
											if ( pcObj->m_bWearPaletteChange != 0)
											{
												for (int i = 0; i < pcObj->m_bWearPaletteChange; i++)
												{
													cWrChange.pasteData((UCHAR*)&pcObj->m_WearVectorPal[i],sizeof(pcItem->m_WearVectorPal[i]));
												}
											}

											//Loop through the avatar's default textures
											if ( m_pcAvatar->m_bBasicTextureChange != 0)
											{
												for (int i = 0; i < m_pcAvatar->m_bBasicTextureChange; i++)
												{
													cWrChange.pasteData((UCHAR*)&m_pcAvatar->m_BasicVectorTex[i],sizeof(m_pcAvatar->m_BasicVectorTex[i]));
												}
											}
											//Loop through the textures of previously equipped items
											for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
											//Loop through the textures of the newly equipped item
											if ( pcItem->m_bWearTextureChange != 0)
											{
												for (int i = 0; i < pcItem->m_bWearTextureChange; i++)
												{
													cWrChange.pasteData((UCHAR*)&pcItem->m_WearVectorTex[i],sizeof(pcItem->m_WearVectorTex[i]));
												}
											}

											//The avatar always consists of seveteen (0x11) submodels. Only the model of the top- or outer-most
											//object is sent. Similarly, it must be checked which if any avatar sub-models are covered by item 
											//models as well as which item models are covered by other item models.
											bool modelIsCovered;

											if ( m_pcAvatar->m_bBasicModelChange != 0) 
											{
												//Loop through the avatar's default models
												//Do not include avatar models superceded by item models
												for (int i = 0; i < m_pcAvatar->m_bBasicModelChange; i++)
												{
													modelIsCovered = false;

													//Loop through the models of previously equipped items
													for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
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
																	if(m_pcAvatar->m_BasicVectorMod[i].m_bModelIndex == pcItemInv->m_WearVectorMod[j].m_bModelIndex)
																	{
																		modelIsCovered = true;
																	}
																}
															}
														}
													}
													//Loop through the models of the newly equipped item
													for (int j = 0; j < pcItem->m_bWearModelChange; j++)
													{
														//Find whether the item affects the given avatar body part
														//If so, the item model index will equal the given avatar model index
														if(m_pcAvatar->m_BasicVectorMod[i].m_bModelIndex == pcItem->m_WearVectorMod[j].m_bModelIndex)
														{
															modelIsCovered = true;
														}
													}

													if (!modelIsCovered)
														cWrChange.pasteData((UCHAR*)&m_pcAvatar->m_BasicVectorMod[i],sizeof(m_pcAvatar->m_BasicVectorMod[i]));
												}
											}
											//Loop through the models of currently equipped items
											//If two items cover the same area, the one with a higher coverage value supercedes
											for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
											{
												if ( ( *itObject )->m_fEquipped == 2 )
												{
													cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
													if ( pcItemInv->m_bWearModelChange != 0)
													{
														for (int i = 0; i < pcItemInv->m_bWearModelChange; i++)
														{
															modelIsCovered = false;
															//Compare the inventory item against all other inventory items
															for ( iterObject_lst itObject2 = m_pcAvatar->m_lstInventory.begin( ); itObject2 != m_pcAvatar->m_lstInventory.end( ); ++itObject2 )
															{
																if ( ( *itObject2 )->m_fEquipped == 2 )
																{
																	cItemModels *pcItemInv2 = cItemModels::FindModel(( *itObject2 )->GetItemModelID());
																	if ( pcItemInv2->m_bWearModelChange != 0)
																	{
																		for (int j = 0; j < pcItemInv2->m_bWearModelChange; j++)
																		{
																			//Compare the current inventory item against another inventory item
																			if (pcItemInv->m_WearVectorMod[i].m_bModelIndex == pcItemInv2->m_WearVectorMod[j].m_bModelIndex)
																				if (pcItemInv->m_dwCoverage < pcItemInv2->m_dwCoverage)
																					modelIsCovered = true;
																		}
																	}
																}
															}
															for (int j = 0; j < pcItem->m_bWearModelChange; j++)
															{
																//Compare the inventory item against the newly equipped item
																if (pcItemInv->m_WearVectorMod[i].m_bModelIndex == pcItem->m_WearVectorMod[j].m_bModelIndex)
																	if (pcItemInv->m_dwCoverage < pcItem->m_dwCoverage)
																		modelIsCovered = true;
															}
															if (!modelIsCovered)
																cWrChange.pasteData((UCHAR*)&pcItemInv->m_WearVectorMod[i],sizeof(pcItemInv->m_WearVectorMod[i]));
														}
													}
												}
											}
											//Loop through the models of the newly equipped item
											for (int i = 0; i < pcItem->m_bWearModelChange; i++)
											{
												modelIsCovered = false;
												//Loop through the models of currently equipped items
												for ( itObject = m_pcAvatar->m_lstInventory.begin( ); itObject != m_pcAvatar->m_lstInventory.end( ); ++itObject )
												{
													if ( ( *itObject )->m_fEquipped == 2 )
													{
														cItemModels *pcItemInv = cItemModels::FindModel(( *itObject )->GetItemModelID());
														if ( pcItemInv->m_bWearModelChange != 0)
														{
															for (int j = 0; j < pcItemInv->m_bWearModelChange; j++)
															{
																//Compare the inventory item against the newly equipped item
																if (pcItem->m_WearVectorMod[i].m_bModelIndex == pcItemInv->m_WearVectorMod[j].m_bModelIndex)
																	if (pcItem->m_dwCoverage < pcItemInv->m_dwCoverage)
																		modelIsCovered = true;
															}
														}
													}
												}
												if (!modelIsCovered)
													cWrChange.pasteData((UCHAR*)&pcItem->m_WearVectorMod[i],sizeof(pcItem->m_WearVectorMod[i]));
											}
											
											pcObj->m_fEquipped = 2;

											char	szCommand[255];
											RETCODE	retcode;
											sprintf( szCommand, "UPDATE items_instance_inventory SET Equipped = %d WHERE GUID = %lu AND OwnerGUID = %lu;",pcObj->m_fEquipped,dwObjectGUID,m_pcAvatar->GetGUID() );
											retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
											retcode = SQLExecute( cDatabase::m_hStmt );											CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
											retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );								CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

											cWrChange.pasteAlign(4);
											cWrChange	<< m_pcAvatar->m_wNumLogins;
											cWrChange	<< ++m_pcAvatar->m_wModelSequence;

											cWorldManager::SendToAllInFocus(m_pcAvatar->m_Location,cWrChange,3);
											/*
											cMessage cmEquip2; // First 022D Set Container to 0
											cmEquip2 << 0x022DL << ++m_pcAvatar->m_bWearSeq << dwObjectGUID << 0x02L << 0x0L;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmEquip2, 3 );

											cMessage cmEquip3; // Second 022D Set Wielder to Avatar's
											cmEquip3 << 0x022DL << ++m_pcAvatar->m_bWearSeq << dwObjectGUID << 0x03L << m_pcAvatar->m_dwGUID;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmEquip3, 3 );
											
											cMessage cmEquip229; // 0229 Set Coverage
											cmEquip229	<< 0x0229L << ++m_pcAvatar->m_bWearSeq << dwObjectGUID << 0x0AL << dwCoverage;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmEquip229, 3 );
											
											cMessage cmActionComplete;
											cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
											AddPacket(WORLD_SERVER,cmActionComplete,4);
											*/
										 }

										//If the object is a weapon (sword, wand, bow, etc.)
										if ((dwCoverage == 0x00100000) || (dwCoverage == 0x01000000))
										{
											cMessage cmWieldObject;
											cmWieldObject	<< 0xF749L << m_pcAvatar->GetGUID( ) << dwObjectGUID << 1L << 1L << m_pcAvatar->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmWieldObject, 3 );
											AddPacket( WORLD_SERVER, cmWieldObject, 4 );
											
											m_pcAvatar->myWeapon = reinterpret_cast <cWeapon *>(m_pcAvatar->FindInventory(dwObjectGUID));
											m_pcAvatar->myWeapon->m_fEquipped = 1;
										}
										//Shields
										else if (dwCoverage == 0x00200000)
										{
											cMessage cmWieldObject;
											cmWieldObject	<< 0xF749L << m_pcAvatar->GetGUID( ) << dwObjectGUID << 3L << 6L << m_pcAvatar->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmWieldObject, 3 );
											AddPacket( WORLD_SERVER, cmWieldObject, 4 );

											m_pcAvatar->myShield = reinterpret_cast <cShield *>(m_pcAvatar->FindInventory(dwObjectGUID));
											m_pcAvatar->myShield->m_fEquipped = 1;
											m_pcAvatar->Armor_Level += m_pcAvatar->myShield->m_dwArmorLevel;
										}
										//Missile Weapons
										else if (dwCoverage == 0x00400000)
										{
											cMessage cmWieldObject;
											cmWieldObject	<< 0xF749L << m_pcAvatar->GetGUID( ) << dwObjectGUID << 2L << 3L << m_pcAvatar->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmWieldObject, 3 );
											AddPacket( WORLD_SERVER, cmWieldObject, 4 );

											pcMW = reinterpret_cast <cAmmo *>(m_pcAvatar->FindInventory(dwObjectGUID));
											pcMW->m_fEquipped = 1;
										}
										break;
									}
									//If the object is found in the world (non-inventory)
									if ( ( pcObj = cWorldManager::FindObject( dwObjectGUID ) ) )
									{
										pcObj->m_fEquipped = 1;

										unsigned char ucAnimBendDown[28] = {
											0x4C, 0xF7, 0x00, 0x00, 0xD6, 0xFD, 0x09, 0x50, 0x08, 0x00, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00,   //(L÷___ý_P________) - 0000
											0x00, 0x00, 0x3D, 0x00, 0x02, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,                           //(__=_________)     - 0010
										};

										cWorldManager::SendToAllInFocus(m_pcAvatar->m_Location,m_pcAvatar->SoundEffect(122),3);

										cMessage cmEquip;
										cmEquip << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x23L << dwObjectGUID << dwCoverage;
										AddPacket( WORLD_SERVER, cmEquip, 4 );
										
										unsigned char ucAnimStandUp[24] = {
											0x4C, 0xF7, 0x00, 0x00, 0xD6, 0xFD, 0x09, 0x50, 0x08, 0x00, 0x08, 0x00, 0x07, 0x00, 0x00, 0x00,   //(L÷___ý_P________) - 0000
											0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00,                                                   //(__=_____)         - 0010
										};
										
										cWorldManager::SendToAllInFocus(m_pcAvatar->m_Location,m_pcAvatar->SoundEffect(119),3);

										cMessage cmMoveObjectToInventory;
										cmMoveObjectToInventory << 0xF74AL << dwObjectGUID << pcObj->m_wNumLogins << ++pcObj->m_wPositionSequence;
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmMoveObjectToInventory, 3 );
										cWorldManager::RemoveObject( pcObj, FALSE, FALSE );
										m_pcAvatar->AddInventory( pcObj );									

										DWORD dwSequenceB = 0;
										BYTE bSequenceA = 0;

										bSequenceA = ++pcObj->m_bInventorySequence;
										dwSequenceB = bSequenceA + 1;
										
										//k109:  Equip item by picking it up from the ground to your weapon slot.
										if ((dwCoverage == 0x00100000) || (dwCoverage == 0x01000000))
										{
											//Wield Object
											cMessage cmWieldObject;
											cmWieldObject	<< 0xF749L << m_pcAvatar->GetGUID( ) << dwObjectGUID << 1L << 1L << m_pcAvatar->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmWieldObject, 3 );

											//pcW = reinterpret_cast <cWeapon *>(m_pcAvatar->FindInventory(dwObjectGUID));
											m_pcAvatar->myWeapon = reinterpret_cast <cWeapon *>(m_pcAvatar->FindInventory(dwObjectGUID));
											//m_pcAvatar->myWeapon = pcW;
											m_pcAvatar->myWeapon->m_fEquipped = 1;
										}
										//Shields
										else if (dwCoverage == 0x00200000)
										{
											cMessage cmWieldObject;
											cmWieldObject	<< 0xF749L << m_pcAvatar->GetGUID( ) << dwObjectGUID << 3L << 6L << m_pcAvatar->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmWieldObject, 3 );
											AddPacket( WORLD_SERVER, cmWieldObject, 4 );

											m_pcAvatar->myShield = reinterpret_cast <cShield *>(m_pcAvatar->FindInventory(dwObjectGUID));
											m_pcAvatar->myShield->m_fEquipped = 1;
											m_pcAvatar->Armor_Level += m_pcAvatar->myShield->m_dwArmorLevel;
										}
										//Missile Weapons
										else if (dwCoverage == 0x00400000)
										{
											cMessage cmWieldObject;
											cmWieldObject	<< 0xF749L << m_pcAvatar->GetGUID( ) << dwObjectGUID << 2L << 3L << m_pcAvatar->m_wNumLogins << ++pcObj->m_wPositionSequence;
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmWieldObject, 3 );
											AddPacket( WORLD_SERVER, cmWieldObject, 4 );

											pcMW = reinterpret_cast <cAmmo *>(m_pcAvatar->FindInventory(dwObjectGUID));
											pcMW->m_fEquipped = 1;
										}

									}
									break;	// case 0x001A
								}
							
								//Dropping an object
								case INVENTORY_DROP:		//0x001B
								{
									DWORD dwObjectGUID = *( DWORD * )&pbData[12];
									if ( !dwObjectGUID )
										break;	//case 0x001B

									cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );
									if ( !pcObj )
										break;	//case 0x001B
									
									//Bend Down Animation
									unsigned char ucAnimBendDown[28] = {
										0x4C, 0xF7, 0x00, 0x00, 0xFF, 0xF2, 0x09, 0x50, 0x3C, 0x00, 0x0A, 0x00, 0x04, 0x00, 0x00, 0x00,   //(L÷_____P________) - 0000
										0x00, 0x00, 0x3D, 0x00, 0x02, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,                           //(__=_________)     - 0010
									};
									CopyMemory( &ucAnimBendDown[4], &m_pcAvatar->m_dwGUID, 4 );
									CopyMemory( &ucAnimBendDown[8], &m_pcAvatar->m_wNumLogins, 2 );
									CopyMemory( &ucAnimBendDown[10], &( ++m_pcAvatar->m_wCurAnim ), 2 );
									CopyMemory( &ucAnimBendDown[12], &( ++m_pcAvatar->m_wMeleeSequence ), 2 );
									cMessage cmBendDownAnim;
									cmBendDownAnim.CannedData( ucAnimBendDown, 28 );
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmBendDownAnim, 3 );

									//Drop From Inventory
									cMessage cmDropFromInventory;
									cmDropFromInventory << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x019AL << dwObjectGUID;
									AddPacket( WORLD_SERVER, cmDropFromInventory, 4 );
									
									//Sound
									unsigned char ucSound[16] = {
										0x50, 0xF7, 0x00, 0x00, 0xFF, 0xF2, 0x09, 0x50, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,   //(P÷_____P_______?) - 0000
									};
									CopyMemory( &ucSound[4], &m_pcAvatar->m_dwGUID, 4 );
									cMessage cmSound;
									cmSound.CannedData( ucSound, 16 );
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmSound, 3 );	
									
									//Standup Animation
									unsigned char ucAnimStand[24] = {
										0x4C, 0xF7, 0x00, 0x00, 0xFF, 0xF2, 0x09, 0x50, 0x3C, 0x00, 0x0B, 0x00, 0x05, 0x00, 0x00, 0x00,   //(L÷_____P________) - 0000
										0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00,                                                   //(__=_____)         - 0010
									};
									CopyMemory( &ucAnimStand[4], &m_pcAvatar->m_dwGUID, 4 );
									CopyMemory( &ucAnimStand[8], &m_pcAvatar->m_wNumLogins, 2 );
									CopyMemory( &ucAnimStand[10], &(++m_pcAvatar->m_wCurAnim), 2 );
									CopyMemory( &ucAnimStand[12], &(++m_pcAvatar->m_wMeleeSequence), 2 );
									cMessage cmAnimStand;
									cmAnimStand.CannedData( ucAnimStand, 24 );
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmAnimStand, 3 );								
									
									//Set Position
									BYTE MV[] = {
										0x48, 0xF7, 0x00, 0x00, 0xF9, 0x6C, 0xD3, 0x95, 0x36, 0x00, 0x00, 0x00, 0x6C, 0x01, 0x7E, 0x01,
										0xF7, 0x4F, 0xCF, 0x41, 0x03, 0xA6, 0xE0, 0xC2, 0xC5, 0x20, 0x30, 0x3D, 0xB9, 0x7E, 0x0D, 0xBF,
										0xC4, 0x57, 0x55, 0xBF, 0x65, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x09, 0x00, 0x02, 0x00, 0x00, 0x00
									};
									CopyMemory( &MV[4], &dwObjectGUID, 4 );
									CopyMemory( &MV[12], &m_pcAvatar->m_Location, 20 );
									CopyMemory( &MV[32], &m_pcAvatar->m_Location.m_flW, 4 );
									CopyMemory( &MV[0x28], &m_pcAvatar->m_wNumLogins, 2 );
									CopyMemory( &MV[0x2A], &( pcObj->m_wPositionSequence += 4 ), 2 );
									CopyMemory( &MV[0x2C], &( ++pcObj->m_wNumPortals ), 2 );
									cMessage cmSetPosition;
									cmSetPosition.CannedData( MV, sizeof( MV ) );
									
									pcObj->SetLocation( m_pcAvatar->m_Location );
									m_pcAvatar->RemoveInventory( pcObj );

									cDatabase::RemoveFromInventoryDB(m_pcAvatar->GetGUID(), pcObj->GetGUID());
										
									cWorldManager::AddObject( pcObj, FALSE );
									cWorldManager::SendToOthersInFocus( m_pcAvatar->m_Location, this, pcObj->CreatePacket( ), 3 ); 
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmSetPosition, 3 );
									
									break;	//case 0x001B
								}
								
								case SWEAR_ALLEGIANCE:		//0x001D
								{
									DWORD dwSwearToPlayerGUID;
									CopyMemory( &dwSwearToPlayerGUID, &pbData[12], 4 );

									bool canSwear = false;

									// If the avatar is in an allegiance
									if (m_pcAvatar->m_dwAllegianceID != 0)
									{
										cAllegiance* aAllegiance = cAllegiance::GetAllegianceByID(m_pcAvatar->GetAllegianceID());
										if (aAllegiance)
										{
											Member* thisMember = aAllegiance->FindMember(m_pcAvatar->GetGUID());
											if (thisMember)
												if (thisMember->m_dwPatron == 0)	// if the avatar has no patron
													canSwear = true;
										}
									}

									// If the avatar is not in an allegiance
									if (m_pcAvatar->m_dwAllegianceID == 0)
									{
										canSwear = true;
									}

									if (canSwear)
									{
										// Find the recipient
										cClient *pcRecvClient = cClient::FindClient( dwSwearToPlayerGUID );
										if (pcRecvClient)
										{
											cAvatar *pc_RecvAvatar = pcRecvClient->m_pcAvatar;
											if (pc_RecvAvatar)
											{
												// Open the recipient avatar's confirmation panel
												pcRecvClient->AddPacket( WORLD_SERVER, pc_RecvAvatar->ConfirmPanelRequest(++m_dwF7B0Sequence,1,cWorldManager::NewConfirmSeq(),m_pcAvatar->Name()), 4 );
												// Add the allegiance request to the pending confirmation queue
												cWorldManager::AddPendingConfirm(cWorldManager::CurrentConfirmSeq(),1,m_pcAvatar->Name(),m_pcAvatar->GetGUID(),pc_RecvAvatar->GetGUID());
											}
										}
									}
									break;
								}
								
								case BREAK_ALLEGIANCE:		//0x001E
								{
									DWORD dwPatronGUID = MType3;
									m_pcAvatar->BreakAllegiance(dwPatronGUID);
									break;
								}
								
								case ALLEGIANCE_PANEL:		//0x001F
								{
									//cMessage cmAllegiance;
									//cmAllegiance << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0020L << 0L << 0L << 0L;
									//AddPacket(WORLD_SERVER,cmAllegiance,4);
									AddPacket(WORLD_SERVER,m_pcAvatar->AllegianceInfo(++m_dwF7B0Sequence),4);

									cMessage cmActionComplete;
									cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
									AddPacket( WORLD_SERVER,cmActionComplete,4);
									break;
								}
								
								//Direct /tells to known avatars
								case SEND_TELL_GUID: 		//0x0032
								{
									BYTE *bTextLength = &pbData[12];
									WORD wTextLength = pbData[12];
									char *szMessageText = new char[wTextLength];
									CopyMemory( szMessageText, &pbData[14], wTextLength );
									szMessageText[wTextLength] = '\0';
									//( char * )&bTextLength[2]
									cMasterServer::SendTell( szMessageText, cClient::FindClient( *(DWORD *)(pbData + ((FH.m_wFragmentLength - sizeof( cFragmentHeader )) - sizeof( DWORD ))) ), this );
									break;	// case 0x0032
								}
								
								case TARGET_USE:			//0x0035
								{
									cObject *pcObj = m_pcAvatar->FindInventory( MType3 );

									cObject* pcTargetObj = cWorldManager::FindObject( MType3 );

									if( pcObj )
									{
										pcObj->Action(this);

										cMessage cmActionComplete;
										cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
										AddPacket( WORLD_SERVER,cmActionComplete,4);
										break;
									}
									else
									{
										//do nothing!
										break;
									}

								}

								case USE:					//0x0036
								{
									//TODO:  Add item type implementation here.

									//Outside inventory
									DWORD dwTargetGUID =  *( DWORD * )&pbData[12];

									//Inside inventory
									DWORD dwObjectGUID = *( DWORD * )&pbData[12];

									NPCID = *( DWORD * )&pbData[12];

									CORPSEID = *( DWORD * )&pbData[12];

									cObject* pcTargetObj = cWorldManager::FindObject( dwTargetGUID );
									cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );
	
									if( pcObj )
									{
										pcObj->Action(this);
										cMessage cmActionComplete;
										cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
										AddPacket( WORLD_SERVER,cmActionComplete,4);
										break;
									}

									if( pcTargetObj )
									{
									
										intRange = m_pcAvatar->GetRange( m_pcAvatar->m_Location.m_dwLandBlock,m_pcAvatar->m_Location.m_flX,m_pcAvatar->m_Location.m_flY,m_pcAvatar->m_Location.m_flZ, pcTargetObj->m_Location.m_dwLandBlock, pcTargetObj->m_Location.m_flX, pcTargetObj->m_Location.m_flY, pcTargetObj->m_Location.m_flZ );
																			
										//debug
										#ifdef _DEBUG
											char szPacket[60];
											sprintf( szPacket, "Range is: %f",intRange);
											cMasterServer::ServerMessage( ColorYellow,this,(char *)szPacket);
										#endif

										if (intRange < 0.5)
										{
											pcTargetObj->Action(this);
											break;
										}
										else
										{
											//pcTargetObj->Assess(this);
											char szPacket[60];
											sprintf( szPacket, "That object is to far away to use!");
											cMasterServer::ServerMessage( ColorYellow,this,(char *)szPacket);
											cMessage cmActionComplete;
											cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
											AddPacket( WORLD_SERVER,cmActionComplete,4);
											break;

										}
									}

									else
									{
										break;
									}
								break;

								}
								
								case RAISE_VITAL:			//0x0044
								{
									switch(MType3)
									{
										case 0x00000001:	//Health
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->RaiseHealth(MType4),4);
												//The client automatically updates the displayed health value.
											}
											break;
										case 0x00000003:	//Stamina
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->RaiseStamina(MType4),4);
												//The client automatically updates the displayed stamina value.
											}
											break;
										case 0x00000005:	//Mana
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->RaiseMana(MType4),4);
												//The client automatically updates the displayed mana value.
											}
											break;
									}
									//Raising each vital consumes experience, thereby reducing unassigned experience
									//Send the Unassigned Experience message to update the client.
									AddPacket( WORLD_SERVER, m_pcAvatar->DecrementUnassignedExp(MType4), 4 );
									break;

								}
								
								case RAISE_ATTRIBUTE:		//0x0045
								{
									switch(MType3)
									{
										case 0x00000001:	//Strength
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateStrength(MType4),4);
											}
											break;
										case 0x00000002:	//Endurance
											{	
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateEndurance(MType4),4);	
												//When endurance is raised:
												//		1.  Stamina should always increase by 1.
												//		2.  Health should increase by 1 for every 2 points spent.
												//The client automatically updates these values as needed.
											}
											break;
										case 0x00000003:	//Quickness
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateQuickness(MType4),4);
											}
											break;
										case 0x00000004:	//Coordination
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateCoordination(MType4),4);
											}
											break;
										case 0x00000005:	//Focus
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateFocus(MType4),4);
											}
											break;
										case 0x00000006:	//Self
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSelf(MType4),4);
												//When self is raised, mana should always increase by 1.
												//The client automatically updates this value as needed.
											}
											break;
									}
									//Raising each attribute consumes experience, thereby reducing unassigned experience
									//Send the Unassigned Experience message to update the client.
									AddPacket( WORLD_SERVER, m_pcAvatar->DecrementUnassignedExp(MType4), 4 );
									break;									
								}
								
								case RAISE_SKILL:			//0x0046
								{
									//Update the skills
									switch(MType3)
									{
										case AXE:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case BOW:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case CROSSBOW:	
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case DAGGER:	
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case MACE:	
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case MELEE_DEFENSE:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case MISSILE_DEFENSE:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case SPEAR:	
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case STAFF:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case SWORD:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case THROWN_WEAPONS:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case UNARMED_COMBAT:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ARCANE_LORE:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case MAGIC_DEFENSE:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;		
										case MANA_CONVERSION:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ITEM_TINKERING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ASSESS_PERSON:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;	
										case DECEPTION:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case HEALING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case S_JUMP:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case LOCKPICK:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case RUN:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ASSESS_CREATURE:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case WEAPON_TINKERING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ARMOR_TINKERING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case MAGIC_ITEM_TINKERING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case CREATURE_ENCHANTMENT:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ITEM_ENCHANTMENT:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case LIFE_MAGIC:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case WAR_MAGIC:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case LEADERSHIP:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case LOYALTY:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case FLETCHING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case ALCHEMY:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case COOKING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;
										case SALVAGING:
											{
												AddPacket( WORLD_SERVER,m_pcAvatar->UpdateSkill(MType3,MType4),4);
											}
											break;								
										}
										//Raising each skill consumes experience, thereby reducing unassigned experience
										//Send the Unassigned Experience message to update the client.
										AddPacket( WORLD_SERVER, m_pcAvatar->DecrementUnassignedExp(MType4), 4 );
										break;
								}
								
								case TRAIN_SKILL:			//0x0047
								{
									break;
								}
								
								case CAST_SPELL_NO_TARGET:	//0x0048
								{
								#ifdef _DEBUG
									char szPacket[60];
									sprintf( szPacket, "F7B1 Packet: %08x",MType2);
									cMasterServer::ServerMessage( ColorYellow,this,(char *)szPacket);
                                #endif
                                break;
								}
								
								//The client performs a magic attack
								case CAST_SPELL_TARGET: 	//0x004A
								{
									DWORD *pdwData = (DWORD *)pbData;
									DWORD	dwMagTarget;
									DWORD	dwSpellID;
									float	flTargetHeading;
									DWORD	dwSequenceSpell;
									int		iTurnTime;

									dwSequenceSpell = pdwData[1];
									dwMagTarget = pdwData[3];
									dwSpellID = pdwData[4];
									
									if ( m_pcAvatar->m_fIsCasting )
									{
										cMessage cmBusy;
										cmBusy << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x028AL << 0x001DL;
										AddPacket( WORLD_SERVER,cmBusy,4);
										cMessage cmActionComplete;
										cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x01C7L << 0L;
										AddPacket( WORLD_SERVER,cmActionComplete,4);
										break;
									}

									m_pcAvatar->m_fIsCasting = true;

									#ifdef _DEBUG
										char szPacketCast[100];
										sprintf( szPacketCast, "Spell Sequence: %d -- Spell Casting: %d ",dwSequenceSpell,dwSpellID);
										cMasterServer::ServerMessage( ColorGreen,this,(char *)szPacketCast);
									#endif
									
									cClient* pcTargetObj = cClient::FindClient( dwMagTarget );

									cSpell *pcSpell = cSpell::FindSpell( dwSpellID );
									if (pcSpell)
									{
										int iNewMana;
										iNewMana = m_pcAvatar->m_cStats.m_lpcVitals[2].m_lTrueCurrent;
										iNewMana -= pcSpell->m_dwManaCost;
										if (iNewMana < 0)
											iNewMana = 0;
										cMessage cmManaLoss = m_pcAvatar->DecrementMana( pcSpell->m_dwManaCost, iNewMana );
										this->AddPacket( WORLD_SERVER, cmManaLoss, 4 );
									}


									if ( pcSpell->m_flUnkFloat1 != 0 )
									{
										//If no client is found (in which case the target is NOT another player)
										if( !pcTargetObj )
										{
											cObject* pcTargetObject = cWorldManager::FindObject( dwMagTarget );
											if( !pcTargetObject )
											{
												flTargetHeading = 0.0f;
											}
											else
											{
												flTargetHeading = cPhysics::GetHeadingTarget(m_pcAvatar->m_Location,pcTargetObject->m_Location);
												
												#ifdef _DEBUG
													sprintf( szPacketCast, "TargetHeading: %f, flA: %f, flW: %f, Heading: %f",flTargetHeading, m_pcAvatar->m_Location.m_flA, m_pcAvatar->m_Location.m_flW, cPhysics::GetAvatarHeading( m_pcAvatar->m_Location ) );
													cMasterServer::ServerMessage( ColorGreen,this,(char *)szPacketCast);
												#endif
												
												cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->TurnToTarget(flTargetHeading, pcTargetObject->GetGUID() ), 3 );
											}
										}
										//If a client is found (in which case the target is another player)
										else
										{
											flTargetHeading = cPhysics::GetHeadingTarget(m_pcAvatar->m_Location,pcTargetObj->m_pcAvatar->m_Location);
											
											cMasterServer::ServerMessage( ColorGreen,this,"Client found.");
											cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, m_pcAvatar->TurnToTarget(flTargetHeading, pcTargetObj->m_pcAvatar->GetGUID() ), 3 );
										
											WORD wTotalOpponentHealth = pcTargetObj->m_pcAvatar->GetTotalHealth();
											//Now perform a very basic combat calculation..no where near accurate.

											DWORD dwDamage = 10;
											DWORD dwDamageType = 0x04;

											double dSeverity = ( double ) wTotalOpponentHealth / dwDamage;

											int iNewHealth;
											BOOL fKilledPlayer = FALSE;
								
											//Following var(s) only used if there is a kill
											cMessage cmAnim;
											//error here
											cMessage cmBeginMessage = m_pcAvatar->AttackBeginMessage( ++m_dwF7B0Sequence, m_pcAvatar->GetGUID( ) );
											cMessage cmHealthLoss = pcTargetObj->m_pcAvatar->DecrementHealth( dwDamage, iNewHealth );
											if( iNewHealth <= 0 ) 
											{
												fKilledPlayer = TRUE;

												cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0), 3 );
												
												cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->Animation( 0x11L, 2.0f ), 3 );		
																		
												cMasterServer::Corpse( pcTargetObj );

												cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0), 3 );
												cWorldManager::TeleportAvatar( pcTargetObj, cMasterServer::m_StartingLoc );
												
												char szDeathNotice[200];
												wsprintf( szDeathNotice, "%s has killed %s", m_pcAvatar->m_strName.c_str(), pcTargetObj->m_pcAvatar->m_strName.c_str( ) );

												pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->SetHealth( pcTargetObj->m_pcAvatar->GetTotalHealth( ) ), 4 );
												
											}
											else
											{
												AddPacket(WORLD_SERVER,m_pcAvatar->CombatAnimation( dwMagTarget ),4);
												pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->RecieveDamageMessage( ++pcTargetObj->m_dwF7B0Sequence, m_pcAvatar->m_strName, dwDamageType, dSeverity, dwDamage, 0x7 ), 4 );
												AddPacket( WORLD_SERVER, m_pcAvatar->DoDamageMessage( ++m_dwF7B0Sequence, pcTargetObj->m_pcAvatar->m_strName, dwDamageType, dSeverity, dwDamage), 4 );
											}

										}
										
										
										float flUserHeading = cPhysics::GetAvatarHeading( m_pcAvatar->m_Location );
										
										iTurnTime = cPhysics::GetHeadingDifference( flUserHeading, flTargetHeading ) / 12 + 2;
									}
									else
									{
										iTurnTime = 1;
										dwMagTarget = m_pcAvatar->GetGUID();
									}

									cWarJobParam* WarJobParam = new cWarJobParam( dwSpellID, m_pcAvatar->GetGUID(), dwMagTarget, m_pcAvatar->m_Location, dwSequenceSpell );

									int iJob = cMasterServer::m_pcJobPool->CreateJob( &cAvatar::WarAnimation1, (LPVOID) WarJobParam, NULL, "WarAnimation1", iTurnTime, 1);
									
									break;
								}
								
								case CHANGE_COMBAT_MODE: 	//0x0053
								{

									if ( pbData[12] == 0x02 )		//Enter Melee Mode
									{
										
										if(!pcW)
										{
										cMessage Anim = m_pcAvatar->ChangeCombatMode( TRUE, 0);
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 3 );
										}
										else if(pcW->m_fEquipped == 0)
										{
										cMessage Anim = m_pcAvatar->ChangeCombatMode( TRUE, 0);
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 3 );
										}
										else
										{
										cMessage Anim = m_pcAvatar->ChangeCombatMode( TRUE, pcW->m_bWieldType);
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 3 );
										}
									}
									else if ( pbData[12] == 0x03 ) //Enter Ranged Combat Mode
									{
										cMessage Anim = m_pcAvatar->ChangeMissileMode( TRUE, 0x02);
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 3 );
									}
									else if ( pbData[12] == 0x04 ) //Enter Spell Mode
									{
										cMessage Anim = m_pcAvatar->ChangeSpellMode( TRUE );
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 3 );
									}
									else if ( pbData[12] == 0x01 )	//Leave combat
									{
										cMessage Anim = m_pcAvatar->ChangeCombatMode( FALSE, 0 );
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, Anim, 3 );
										
										cMessage cmCompleteAttackMessage = m_pcAvatar->AttackCompleteMessage( ++m_dwF7B0Sequence );
										AddPacket( WORLD_SERVER, cmCompleteAttackMessage, 4 );
									}
									break;
								}
								
								case STACK_ITEMS:			//0x0054
								{

									break;
								}
								
								case SPLIT_ITEMS:			//0x0055
								{
									//msg format:
									//DWORD Item GUID of original item
									//DWORD Avatar GUID
									//DWORD Pack Slot (this is returned in the F7B0 message)
									//DWORD New stack value
	
									cObject *findItem = m_pcAvatar->FindInventory(MType3);
									cItemModels *pcModel = cItemModels::FindModel(findItem->GetItemModelID());

									DWORD *pdwData = (DWORD *)pbData;
									DWORD Item_GUID = MType3;
									DWORD Slot = MType4;
									DWORD Value = pbData[24];
									
									if(pcModel->m_ItemType = 9)
									{
										cPyreals *cash = reinterpret_cast<cPyreals *>(findItem);
										cash->Split(this,Item_GUID,Slot,Value);
									}

									break;
								}
								
								case SQUELCH:				//0x0058
								{
									//msg format:
									//DWORD (0x01) - 1 squelched, 0 unsquelched.
									//DWORD (0x00)
									//STRING - Character name
									//DWORD (0x01)

									break;
								}
								
								case SQUELCH_ACCOUNT:		//0x0059
								{
																		//msg format:
									//DWORD (0x01) - 1 squelched, 0 unsquelched.
									//DWORD (0x00)
									//STRING - Character name (align to boundary)

									break;
								}
								
								case SEND_TELL_NAME: 		//0x005D:
								{	// Direct /tells to unknown avatars.
								//	BYTE *bTextLength = &pbData[12], *bNameLength;
									BYTE *bTextLength = &pbData[12], *bNameLength;
									WORD wTextLength = pbData[12];
									char *szMessageText = new char[wTextLength];
									CopyMemory( szMessageText, &pbData[14], wTextLength );
									szMessageText[wTextLength] = '\0';

									bNameLength = bTextLength + 2 + *( ( WORD * )bTextLength) + pcRecvPacket->GetPadding( bTextLength );
									cMasterServer::SendTell( szMessageText, cClient::FindClient( ( char * )&bNameLength[2] ), this );

									//cMasterServer::SendTell( (char *)&bTextLength[2], cClient::FindClient( ( char * )&bNameLength[2] ), this );
									break;	//case 0x005D
								}
								
								case VENDOR_BUY_ITEMS:		//0x005F
								{

									DWORD ItemID = *( DWORD * )&pbData[24];
									cNPC *m_NPC = reinterpret_cast<cNPC *>(cWorldManager::FindObject(MType3));
									m_NPC->BuyItem(this, m_NPC->GetGUID(), ItemID);

									cMessage cmActionComplete;
									cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_pcAvatar->m_dwF7B0Sequence << 0x01C7L << 0L;
									AddPacket( WORLD_SERVER,cmActionComplete,4);

									break;
								}
								
								case VENDOR_SELL_ITEMS:		//0x0060
								{
									break;
								}
								
								case LIFESTONE_RECALL: 		//0x0063
								{
									cMessage cmLSAnim = m_pcAvatar->LSAnimate();
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmLSAnim, 3 );

									cMessage cmLSMessage = m_pcAvatar->LSMessage();
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location,cmLSMessage, 4 );
									m_pcAvatar->m_dwPingCount = 0;
									m_pcAvatar->m_wLifeStone = 1;
									
									break;
								}
								
								//Login
								case CHARACTER_SPAWN: 		//0x00A1
								{
									cWorldManager::AddClient( this );
									break;	// case 0x00A1
								}
								
								case CREATE_FELLOWSHIP:		//0x00A2
								{
									//msg info
									//String Name of Fellow (align to boundary)
									//DWORD (0x01)
									
									WORD	wLength;
									char	strFellowName[50];

									CopyMemory( &wLength, &pbData[12], 2 );
									CopyMemory( strFellowName, &pbData[14], wLength );
									strFellowName[wLength] = '\0';

									m_pcAvatar->CreateFellowship( m_dwF7B0Sequence, strFellowName );

									break;
								}

								case DISBAND_FELLOWSHIP:	//0x00A3
								{
									m_pcAvatar->DisbandFellowship( );

									break;
								}
								
								case DISMISS_FELLOW_MEMBER:	//0x00A4
								{
									DWORD dwDismissPlayerGUID;
									CopyMemory( &dwDismissPlayerGUID, &pbData[12], 4 );

									m_pcAvatar->FellowshipDismiss( dwDismissPlayerGUID );

									break;
								}
								
								case RECRUIT_FELLOW_MEMBER: //0x00A5
								{
									DWORD dwRecruitPlayerGUID;
									CopyMemory( &dwRecruitPlayerGUID, &pbData[12], 4 );

									bool canRecruit = false;

									// If the avatar is in a fellowship
									if (m_pcAvatar->inFellow)
									{
										cFellowship* aFellowship = cFellowship::GetFellowshipByID(m_pcAvatar->m_dwFellowshipID);
										if (aFellowship->GetIsOpen() == true || aFellowship->GetLeader() == m_pcAvatar->GetGUID())
										{
											canRecruit = true;
										}
									}

									if (canRecruit)
									{
										// Find the recipient
										cClient *pcRecvClient = cClient::FindClient( dwRecruitPlayerGUID );
										if (pcRecvClient)
										{
											cAvatar *pc_RecvAvatar = pcRecvClient->m_pcAvatar;
											if (pc_RecvAvatar)
											{
												if (pc_RecvAvatar->m_dwFellowshipID != 0)
												{	// already in a fellowship
												}
												else
												{
													// Open the recipient avatar's confirmation panel
													pcRecvClient->AddPacket( WORLD_SERVER, pc_RecvAvatar->ConfirmPanelRequest(++m_dwF7B0Sequence,4,cWorldManager::NewConfirmSeq(),m_pcAvatar->Name()), 4 );
													// Add the fellowship request to the pending confirmation queue
													cWorldManager::AddPendingConfirm(cWorldManager::CurrentConfirmSeq(),4,m_pcAvatar->Name(),m_pcAvatar->GetGUID(),pc_RecvAvatar->GetGUID());
												}
											}
										}
									}
									break;
								}
								
								case FELLOWSHIP_PANEL:		//0x00A6
								{
									//No meesage is returned from the server for this message.
									break;
								}

								case WRITE_BOOK:			//0x00AB
								{
									//Message format:  GUID, Page number(?), String (text written)
									//F7B0:00B5 directly followed this message: format - GUID, 0x0, pagenumber?
									break;
								}
								
								case UNK:					//0x00AC
								{
									//This message appeared before writing to a book.  
									//the F7B0:00B6 message came directly after.

									//0x00AC Message format: GUID
									//0x00B6 Message format: GUID, 0x0, pagenumber?
								}
								case READ_BOOK:				//0x00AE
								{
									
									cObject *pcObj = m_pcAvatar->FindInventory( MType3 );
									cBooks *book = reinterpret_cast<cBooks *>(pcObj);
									book->Read(this,MType3,MType4);
									
									break;
								}
								
								case INSCRIBE:				//0x00BF
								{	
									//Format is ItemID, string.
								}
								
								case ASSESS:				//0x00C8
								{
									
									cObject *pcObj = m_pcAvatar->FindInventory( MType3 );

									cObject *pcTargetObj = cWorldManager::FindObject( MType3 );
			
									cAvatar *v_Avatar  = cWorldManager::FindAvatar( MType3 );

									if( pcObj )				//If the object was found in the avatar's inventory
									{
										pcObj->Assess(this);
										break;
									}
									else if(pcTargetObj)	//If the object was found in the world (non-inventory)
									{
										pcTargetObj->Assess(this);
										break;
									}
									else if(v_Avatar)		//If the object was another player
									{
										v_Avatar->Assess(this);
									}

									break;
								}
								
								case GIVE_ITEM:				//0x00CD
								{
									#ifdef _DEBUG
									cMasterServer::ServerMessage( ColorYellow,this,"Give item message");
									#endif

									cNPC *m_NPC = reinterpret_cast<cNPC *>(cWorldManager::FindObject(MType3));
									cObject *pcObj = m_pcAvatar->FindInventory(MType4);
									
									cItemModels *pcQuestItem = cItemModels::FindModel(pcObj->GetItemModelID());
									if(!pcQuestItem)
										break; //bail!!

									//Is the client giving the item to a Town Crier?  If so, destroy the item.
									if(m_NPC->GetIsVendor() == 3)
									{
										#ifdef _DEBUG
										cMasterServer::ServerMessage( ColorYellow,this,"Town Crier");
										#endif

										//Town crier will take the item and destroy it!
										m_NPC->GiveItem(this,MType3,MType4);

										cMessage cmActionComplete;
										cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_pcAvatar->m_dwF7B0Sequence << 0x01C7L << 0L;
										AddPacket( WORLD_SERVER,cmActionComplete,4);
										break;
									}
									//Does the NPC accept the item?
									else if(pcQuestItem->m_dwQuestItemID != m_NPC->m_qitem_id1)
									{

										cMasterServer::ServerMessage(ColorYellow,this,"m_dwQuestItemID = %d, m_qitem_id1 = %d",pcQuestItem->m_dwQuestItemID,m_NPC->m_qitem_id1);
										//NPC does not accept this item.
										cMasterServer::ServerMessage(ColorGreen,this,"%s does not know what to do with that",m_NPC->m_strName.c_str());
										
										//Remove the item
										m_pcAvatar->DeleteFromInventory(pcObj);
										cMessage cmRemoveItem;
										cmRemoveItem << 0x0024L << pcObj->GetGUID();
										AddPacket(WORLD_SERVER,cmRemoveItem,4);
										
										//Set Container
										cMessage cmSetContainer;
										cmSetContainer	<< 0x022DL << ++pcObj->m_bInventorySequence << pcObj->GetGUID() << 2L << m_pcAvatar->GetGUID( );
										AddPacket( WORLD_SERVER, cmSetContainer, 4 );

										m_pcAvatar->AddInventory(pcObj);
										//m_pcAvatar->RemoveInventory(pcObj);
										

										//If there are no results, refuse the item
									//	cMessage cmRefuseItem;
									//	cmRefuseItem << 0xF7B0L << m_pcAvatar->GetGUID() << ++m_pcAvatar->m_dwF7B0Sequence << 0x00A0L
									//	<< MType4 << 0x046AL;
									//	AddPacket(WORLD_SERVER,cmRefuseItem,4);
										
									//	m_pcAvatar->DeleteFromInventory(pcObj);
										

										//Failure to give message.
									//	cMessage cmFailToGive;
									//	cmFailToGive << 0xF7B0L << m_pcAvatar->GetGUID() << ++m_pcAvatar->m_dwF7B0Sequence << 0x028BL << 0x046AL << m_NPC->m_strName.c_str();
									//	AddPacket(WORLD_SERVER,cmFailToGive,4);
										
										cMessage cmActionComplete;
										cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_pcAvatar->m_dwF7B0Sequence << 0x01C7L << 0L;
										AddPacket( WORLD_SERVER,cmActionComplete,4);
										break;
									}
									//If the NPC accepts the item
									else
									{
										#ifdef _DEBUG
											cMasterServer::ServerMessage( ColorYellow,this,"Quest NPC");
										#endif

										//continue with give item.
										m_NPC->GiveItem(this,MType3,MType4);

										cMessage cmActionComplete;
										cmActionComplete << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_pcAvatar->m_dwF7B0Sequence << 0x01C7L << 0L;
										AddPacket( WORLD_SERVER,cmActionComplete,4);
										break;
									}

								break;

								}
								
								case REMOVE_PRIVS:			//0x00D3
								{
									break;
								}
								
								case TELE_TO_LB:			//0x00D6
								{
									break;
								}
								
								case REPORT_ABUSE:			//0x0140
								{
									break;
								}
								
								case SEND_TELL_MASK:		//0x0147
								{
									break;
								}
								
								case CLOSE_CONTAINER_FORCED://0x0195
								{
									break;
								}
								
								case MAKE_SHORTCUT:			//0x019C
								{
									break;
								}
								
								case REMOVE_SHORTCUT:		//0x019D
								{
									break;
								}
								
								case ADJUST_SETTINGS:		//0x01A1
								{
								/*
								Character Settings:
									0x00000001 = Automatically Create Shortcuts				checkmarked
									0x00000100 = Display Tooltips							checkmarked
									0x00000800 = Stay in Chat Mode after sending message	checkmarked
									0x00008000 = Vivid Targeting Indicator					checkmarked

									0x00080000 = Accept Corpse-Looting Permissions			checkmarked
									0x00000200 = Attempt to Decieve Other Players			checkmarked
									0x00000040 = Let Other Players Give You Items			checkmarked
									0x00020000 = Ignore All Trade Requests					checkmarked
									0x00000400 = Run as Default Movement					checkmarked
									0x00001000 = Advanced Combat Interface (no Panel)		checkmarked
									0x00002000 = Auto Target								checkmarked
									0x00000002 = Automatically Repeat Attacks				checkmarked
									0x00000080 = Automatically keep combat targets in view	checkmarked
									0x00010000 = Disable Most Weather Effects				checkmarked
									0x02000000 = Disable House Restriction Effects			checkmarked
									0x00000010 = Invert Mouse Look Up/Down					checkmarked
									0x00004000 = Right-click mouselook						checkmarked
									0x00200000 = Stretch UI									checkmarked
									0x00400000 = Show Coordinates Below The Radar			checkmarked
									0x00800000 = Display Spell Durations					checkmarked
									0x01000000 = Play Sounds Only When Active Application	checkmarked
									0x04000000 = Drag item to player opens Secure Trade		checkmarked
									0x08000000 = Show Alliegance Logons						checkmarked
									0x10000000 = Use Charge Attack							checkmarked

								Allegiance Settings:
									0x00000004 = Accept Allegiance Requests					uncheckmarked

								Fellowship Settings:
									0x00000008 = Accept Fellowship Requests					uncheckmarked
									0x00040000 = Share Fellowship Experience				checkmarked
									0x00100000 = Share Fellowship Loot/Treasure				checkmarked
									0x20000000 = Auto-Accept Fellowship Requests			checkmarked

								Trade Settings:
									0x00020000 = Ignore All Trade Requests					checkmarked
								*/

									cDatabase::SaveCharacterFlags(m_pcAvatar->GetGUID(),MType3);
									m_pcAvatar->m_dwOptions = MType3;
									break;
								}
								
								case SAVE_LIFESTONE_POS:	//0x01A2
								{
									break;
								}
								
								case DELETE_SPELL_SHORTCUT:	//0x01A8
								{
									break;
								}
								
								case STOP_ATTACK: 			//0x01B7
								{
									#ifdef _DEBUG
										char szPacket[60];
										sprintf( szPacket, "F7B1 Packet: %08x",MType2);
										cMasterServer::ServerMessage( ColorYellow,this,(char *)szPacket);
									#endif
									break;
								}
								
								case REQUEST_HEALTH_UPDATE: //0x01BF
								{
									DWORD dwTargetGUID =  *( DWORD * )&pbData[12];

									cClient* pcTarget = cClient::FindClient( dwTargetGUID );

									if( !pcTarget )
									{
										cObject* pcTargetObj = cWorldManager::FindObject( dwTargetGUID );										
										if( pcTargetObj )
										{
											cMessage cmHealthBar = pcTargetObj->AdjustBar( m_pcAvatar->GetGUID(), ++m_dwF7B0Sequence );
											AddPacket( WORLD_SERVER, cmHealthBar, 4 );
										}
									}
									else
									{
										cMessage cmHealthLoss = pcTarget->m_pcAvatar->AdjustBar( m_pcAvatar->GetGUID(), ++m_dwF7B0Sequence );
										AddPacket( WORLD_SERVER, cmHealthLoss, 4 );
									}
									break;
								}

								case RETRIEVE_AGE:			//0x01C2
								{
									break;
								}
								
								case RETRIEVE_BIRTH:		//0x01C4
								{
									break;
								}
								
								case EMOTE_TEXT:			//0x01DF
								{
									break;
								}
								
								case EMOTE_COMMAND_TEXT:	//0x01E1
								{
									break;
								}
								
								case ADD_SPELL_SHORTCUT:	//0x01E3
								{
									DWORD _tabid = *(DWORD*)&pbData[20];
									cDatabase::AddSpellToTab (m_pcAvatar->GetGUID(), MType3, MType4, _tabid);
									break;
								}
								
								case REMOVE_SPELL_SHORTCUT:	//0x01E4
								{
									break;
								}
								
								case TELEPORT_TO_PLAYER:	//0x01E6 
								{
									break;
								}
								
								case PING_SERVER:			//0x01E9
								{
									cMessage cmPing;
									cmPing << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_pcAvatar->m_dwF7B0Sequence << 0x01EAL;
									AddPacket( WORLD_SERVER,cmPing,4);
									break;
								}
								
								case BEGIN_TRADE:			//0x01F6
								{
									break;
								}
								
								case END_TRADE:				//0x01F7
								{
									break;
								}
								
								case ADD_ITEM_TO_TRADE:		//0x01F8
								{
									break;
								}
								
								case ACCEPT_TRADE:			//0x01FA
								{
									break;
								}
								
								case WITHDRAW_OFFER:		//0x01FB
								{
									break;
								}
								
								case CLEAR_TRADE_WINDOW:	//0x0204
								{
									break;
								}
								
								case CONSENT_CLEAR:			//0x0216
								{
									break;
								}
								
								case CONSENT_LIST:			//0x0217
								{
									break;
								}
								
								case CONSENT_REMOVE:		//0x0218
								{
									break;
								}
								
								case PERMIT_ADD:			//0x0219
								{
									break;
								}
								
								case PERMIT_REMOVE:			//0x021A
								{
									break;
								}
								
								case HOUSE_BUY:				//0x021C
								{
									//"Congratulations! You now own this dwelling."
									DWORD dwCovenantGUID = MType3;
									int numItems = MType4;

									DWORD dwObjectGUID;
									int objectOffset = 0;

									char	szCommand[512];
									RETCODE	retcode;
									int		index,i;
									BOOLEAN fetchSuccess = false;
									DWORD	dwHouseID;
									char	OwnerIDBuff[9];
									DWORD	OwnerID = NULL;

									DWORD	dwMaintenanceCount;	//Apartment, Cottage = 1; Villa, Mansion = 2
									DWORD	dwMaintainType;
									DWORD	dwMaintainRequired;
									DWORD	dwMaintainPaid;

									DWORD	dwPurchaseCount;	//Apartment = 2; Cottage, Villa = 3; Mansion = 6
									DWORD	dwBuyType;
									DWORD	dwBuyRequired;
									DWORD	dwBuyPaid;

									DWORD	dwHouseType;
									DWORD	dwPurchaseTime;

									char szName[75];
									std::string		strPlayerName = "";

									//While there are items being submitted to purchase the dwelling
									for (index = 0; index < numItems; index++)
									{
										CopyMemory( &dwObjectGUID, &pbData[20 + objectOffset], 4 );
										cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );
										objectOffset += 4;

										sprintf( szCommand, "SELECT HouseID,OwnerID FROM houses_covenants WHERE GUID = '%08x';",dwCovenantGUID);
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHouseID, sizeof( dwHouseID ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
										retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
										retcode = SQLFetch( cDatabase::m_hStmt );
										
										sscanf(OwnerIDBuff,"%08x",&OwnerID);
										
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										
										cMasterServer::ServerMessage(ColorGreen,this,"HouseID: %d", dwHouseID);

										sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_purchase WHERE HouseID = %d AND ItemLinker = %d;",dwHouseID,pcObj->GetItemModelID());
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwBuyType, sizeof( dwBuyType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
										retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwBuyRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwBuyPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										
										if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
										{
											fetchSuccess = true;
										}
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

										//If there are items required to purchase the dwelling
										if (fetchSuccess)
										{
											sprintf( szCommand, "SELECT Name FROM avatar WHERE AvatarGUID = %08x;",OwnerID);
											retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
											retcode = SQLExecute( cDatabase::m_hStmt );
										
											retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, szName, sizeof( szName ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

											//Return SQL_SUCCESS if there is a player that corresponds to owner of the fetched house
											if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
											{
												std::string		str1 = szName;
  												strPlayerName.assign(str1);
											}
											retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
											retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

											//If the required amount left of the item is more than the amount paid
											if (dwBuyRequired > dwBuyPaid)
											{
												int remainingBuy = 0;
												remainingBuy = dwBuyRequired - dwBuyPaid;
												
												if (remainingBuy < pcObj->m_dwQuantity)
												{
													dwBuyPaid += remainingBuy;
													pcObj->m_dwQuantity -= remainingBuy;
												} else {
													dwBuyPaid += pcObj->m_dwQuantity;
													pcObj->m_dwQuantity = 0;
												}
												
												sprintf( szCommand, "UPDATE houses_purchase SET Paid = %d WHERE HouseID = %d AND ItemLinker = %d;",dwBuyPaid,dwHouseID,pcObj->GetItemModelID());
												retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLExecute( cDatabase::m_hStmt );
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												
												/* F7B0:0x0228 Message */
												cMessage cmMaintenance;
												cmMaintenance << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0228L;

												sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLFetch( cDatabase::m_hStmt );
												if( retcode == SQL_NO_DATA )
													dwMaintenanceCount = 0;
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												cmMaintenance << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling
												
												//String maintainName;
												//String maintainPluralName;	
										
												sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );														
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

													cmMaintenance	<< dwMaintainRequired			// quantity required
																	<< dwMaintainPaid				// quantity paid
																	<< DWORD(pcModel->m_wModel)		// item's object type
																	<< pcModel->m_strName.c_str()	// name of this item
																	<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
												}
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
												
												AddPacket( WORLD_SERVER, cmMaintenance, 4 );

												/* F7B0:0x021D Message */
												//Display Dwelling Purchase/Maintenance Panel
												cMessage cmDisplayPanel;
												cmDisplayPanel << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x021DL 
												<< dwCovenantGUID;			// Covenant Crystal's GUID

												DWORD ownerType =		0x00000001;	// Self = 0x00000001; Allegiance = 0x00000003
												DWORD levelReq =		0x00000001;	// Apartment, Cottage = 0x00000014; Villa = 0x00000023; Mansion = 0x00000032
												DWORD unknown2 =		0xFFFFFFFF;
												DWORD rankReq =			0xFFFFFFFF;	// Apartment, Cottage, Villa = 0xFFFFFFFF; Mansion = 0x00000006
												DWORD unknown3 =		0xFFFFFFFF;
												DWORD unknown4 =		0x00000000;
												DWORD unknown5 =		0x00000001;
												DWORD ownerName=		0x00000000;
												DWORD purchaseCount =	0x00000001;	// Apartment = 2; Cottage, Villa = 3; Mansion = 6;
												cmDisplayPanel	<< dwHouseID		// Dwelling ID
																<< OwnerID			// Dwelling Owner ID (NULL == 0x00000000)
																<< ownerType
																<< levelReq			// level requirement to purchase this dwelling (-1 if no requirement)
																<< unknown2
																<< rankReq			// rank requirement to purchase this dwelling (-1 if no requirement)
																<< unknown3
																<< unknown4
												//				<< unknown5
																<< strPlayerName.c_str();	// name of the current owner (NULL == 0x00000000)

												sprintf( szCommand, "SELECT COUNT(ID) FROM houses_purchase WHERE HouseID = %d;",dwHouseID );
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwPurchaseCount, sizeof( dwPurchaseCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLFetch( cDatabase::m_hStmt );
												if( retcode == SQL_NO_DATA )
													dwPurchaseCount = 0;
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												cmDisplayPanel << dwPurchaseCount;		// the number of items required to purchase this dwelling

												//String buyName;
												//String buyPluralName;

												sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_purchase WHERE HouseID = %d;",dwHouseID );														
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwBuyType, sizeof( dwBuyType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
												retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwBuyRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwBuyPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

												for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
												{
													if( retcode == SQL_NO_DATA )
													{
														dwBuyRequired = 1;
														dwBuyPaid = 0;
														dwBuyType = 12;
													}
													cItemModels *pcModel = cItemModels::FindModel(dwBuyType);
													std::string		strPluralName = pcModel->m_strName.c_str();
  													strPluralName.assign(strPluralName + "s");

													cmDisplayPanel	<< dwBuyRequired				// quantity required
																	<< dwBuyPaid					// quantity paid
																	<< DWORD(pcModel->m_wModel)		// item's object type
																	<< pcModel->m_strName.c_str()	// name of this item
																	<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
												}
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

												sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLFetch( cDatabase::m_hStmt );
												if( retcode == SQL_NO_DATA )
													dwMaintenanceCount = 0;
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												cmDisplayPanel << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling
													
												//String maintainName;
												//String maintainPluralName;	
												
												sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );														
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintainType, sizeof( dwMaintainType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
												retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwMaintainRequired, sizeof( dwMaintainRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwMaintainPaid, sizeof( dwMaintainPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

												for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
												{
													if( retcode == SQL_NO_DATA )
													{
														dwMaintainRequired = 1;
														dwMaintainPaid = 0;
														dwMaintainType = 12;
													}
													cItemModels *pcModel = cItemModels::FindModel(dwMaintainType);
													std::string		strPluralName = pcModel->m_strName.c_str();
  													strPluralName.assign(strPluralName + "s");

													cmDisplayPanel	<< dwMaintainRequired			// quantity required
																	<< dwMaintainPaid				// quantity paid
																	<< DWORD(pcModel->m_wModel)		// item's object type
																	<< pcModel->m_strName.c_str()	// name of this item
																	<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
												}
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

												AddPacket(WORLD_SERVER,cmDisplayPanel,4);

												//Alter avatar inventory accordingly
												if (pcObj->m_dwQuantity <= 0)
												{
													//Destroy object
													cMessage cmDestroyObject;
													cmDestroyObject << 0x0024L << dwObjectGUID;
													AddPacket( WORLD_SERVER, cmDestroyObject, 4 );
													m_pcAvatar->RemoveInventory( pcObj );
												} else {
													cMasterServer::ServerMessage(ColorGreen,this,"ItemModelID: %d, ItemQuant: %d",pcObj->GetItemModelID(),pcObj->m_dwQuantity);
													//Adjust stack size
													cMessage cmAdjustStack;
													//cItemModels *pcModel = cItemModels::FindModel( pcObj->m_dwItemModelID );
													cmAdjustStack	<< 0x0197L
																	<< BYTE(0x00)					//BYTE sequence -- Seems to be a sequence number of some sort 
																	<< DWORD(pcObj->GetGUID( ))	//Object item -- Item getting its stack adjusted.
																	<< DWORD(pcObj->m_dwQuantity)	//DWORD count -- New number of items in the stack. 
																	<< DWORD(pcObj->m_dwQuantity);//DWORD (pcObj->m_dwQuantity * pcModel->m_dwValue); //DWORD value -- New value for the item.
													AddPacket( WORLD_SERVER, cmAdjustStack, 4 );
												}
											}
										}
									}

									BOOLEAN purchasePaid = true;
									DWORD	dwRequired;
									DWORD	dwPaid;
									sprintf( szCommand, "SELECT Required,Paid FROM houses_purchase WHERE HouseID = %d;",dwHouseID );														
									retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
										if (dwRequired > dwPaid) purchasePaid = false; // determine whether all items are now paid
									}
									retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
									if (purchasePaid)
									{
										//Update the database records to record the avatar owning the house

										//Update the covenant crystal
										sprintf( szCommand, "UPDATE houses_covenants SET OwnerID = %d WHERE GUID = '%08x';",m_pcAvatar->GetGUID( ),dwCovenantGUID );
										retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLExecute( cDatabase::m_hStmt );
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

										sprintf( szCommand, "SELECT HouseID FROM houses_covenants WHERE GUID = '%08x';",dwCovenantGUID );										
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHouseID, sizeof( dwHouseID ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFetch( cDatabase::m_hStmt );
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										
										//Update the house
										sprintf( szCommand, "UPDATE houses SET OwnerID = %d WHERE wModel = %d;",m_pcAvatar->GetGUID( ),dwHouseID );
										retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLExecute( cDatabase::m_hStmt );
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										
										cMasterServer::ServerMessage(ColorGreen,this,"Congratulations! You now own this dwelling.");	
									
										sprintf( szCommand, "SELECT HouseID,HouseType,PurchaseTime FROM houses_covenants WHERE GUID = '%08x';",dwCovenantGUID );
										
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHouseType, sizeof( dwHouseType ), NULL );			CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwPurchaseTime, sizeof( dwPurchaseTime ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFetch( cDatabase::m_hStmt );

										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )			
										
										/* F7B0:0x0225 Message */
										//House Information for Owners
										cMessage cmHouseInfo;
										cmHouseInfo	<< 0x0225L
													<< dwPurchaseTime
													<< dwPurchaseTime	// maintainence last paid?
													<< dwHouseType		// Cottage = 1; Villa = 2; Mansion = 3; Apartment = 4
													<< DWORD(0x0L);		// unknown (0x00000000)

										DWORD dwPurchaseCount;	// Apartment = 2; Cottage, Villa = 3; Mansion = 6;
										sprintf( szCommand, "SELECT COUNT(ID) FROM houses_purchase WHERE HouseID = %d;",dwHouseID );
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwPurchaseCount, sizeof( dwPurchaseCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLFetch( cDatabase::m_hStmt );
										if( retcode == SQL_NO_DATA )
											dwPurchaseCount = 0;
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
										cmHouseInfo << dwPurchaseCount;

										DWORD dwBuyType;
										DWORD dwBuyRequired;
										DWORD dwBuyPaid;
										//String buyName;
										//String buyPluralName;

										sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_purchase WHERE HouseID = %d;",dwHouseID );														
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

											cmHouseInfo	<< dwBuyRequired				// quantity required
														<< dwBuyPaid					// quantity paid
														<< DWORD(pcModel->m_wModel)		// item's object type
														<< pcModel->m_strName.c_str()	// name of this item
														<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
										}
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

										DWORD dwMaintenanceCount;		// Apartment, Cottage = 1; Villa, Mansion = 2;
										sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLFetch( cDatabase::m_hStmt );
										if( retcode == SQL_NO_DATA )
											dwMaintenanceCount = 0;
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
										cmHouseInfo << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling
										
										DWORD dwMaintainType;
										DWORD dwMaintainRequired;
										DWORD dwMaintainPaid;
										//String maintainName;
										//String maintainPluralName;	
								
										sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );														
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

											cmHouseInfo	<< dwMaintainRequired			// quantity required
														<< dwMaintainPaid				// quantity paid
														<< DWORD(pcModel->m_wModel)		// item's object type
														<< pcModel->m_strName.c_str()	// name of this item
														<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
										}
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

										char		locBuff[9];
										cLocation	locCovenant;
										sprintf( szCommand, "SELECT Landblock,Position_X,Position_Y,Position_Z,Orientation_W,Orientation_X,Orientation_Y,Orientation_Z FROM houses_covenants WHERE HouseID = %d;",dwHouseID );	
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										int iCol = 1;
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_CHAR, locBuff, sizeof( locBuff ), NULL );							CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
	 									retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flX, sizeof( &locCovenant.m_flX ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flY, sizeof( &locCovenant.m_flY ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flZ, sizeof( &locCovenant.m_flZ ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flA, sizeof( &locCovenant.m_flA ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flB, sizeof( &locCovenant.m_flB ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flC, sizeof( &locCovenant.m_flC ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, iCol++, SQL_C_FLOAT, &locCovenant.m_flW, sizeof( &locCovenant.m_flW ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
										{
											sscanf(locBuff,"%08x",&locCovenant.m_dwLandBlock);
										}
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
										cmHouseInfo << locCovenant;
									}

									break;
								}
								
								case HOUSE_MAINTAIN:		//0x0221
								{
									//"Maintenance partially paid."
									//"Maintenance paid."
									//"The rent has already been paid!"

									DWORD dwCovenantGUID = MType3;
									int numItems = MType4;

									DWORD dwObjectGUID;
									int objectOffset = 0;

									char	szCommand[512];
									RETCODE	retcode;
									int		index,i;
									BOOLEAN fetchSuccess = false;
									DWORD	dwHouseID;
									char	OwnerIDBuff[9];
									DWORD	OwnerID = NULL;

									DWORD	dwMaintenanceCount;	//Apartment, Cottage = 1; Villa, Mansion = 2
									DWORD	dwMaintainType;
									DWORD	dwMaintainRequired;
									DWORD	dwMaintainPaid;

									DWORD	dwPurchaseCount;	//Apartment = 2; Cottage, Villa = 3; Mansion = 6
									DWORD	dwBuyType;
									DWORD	dwBuyRequired;
									DWORD	dwBuyPaid;

									char szName[75];
									std::string		strPlayerName = "";

									//While there are items being submitted to maintain the dwelling
									for (index = 0; index < numItems; index++)
									{
										CopyMemory( &dwObjectGUID, &pbData[20 + objectOffset], 4 );
										cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );
										objectOffset += 4;

										sprintf( szCommand, "SELECT HouseID,OwnerID FROM houses_covenants WHERE GUID = '%08x';",dwCovenantGUID);
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwHouseID, sizeof( dwHouseID ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
										retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_CHAR, OwnerIDBuff, sizeof( OwnerIDBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
										retcode = SQLFetch( cDatabase::m_hStmt );
										
										sscanf(OwnerIDBuff,"%08x",&OwnerID);
										
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										
										cMasterServer::ServerMessage(ColorGreen,this,"HouseID: %d", dwHouseID);

										sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d AND ItemLinker = %d;",dwHouseID,pcObj->GetItemModelID());
										retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintainType, sizeof( dwMaintainType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
										retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwMaintainRequired, sizeof( dwMaintainRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwMaintainPaid, sizeof( dwMaintainPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										
										if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
										{
											fetchSuccess = true;
											cMasterServer::ServerMessage(ColorGreen,this,"dwcovenantGUID: %08x, ObjectGUID: %d, ItemLinker: %d, ItemModelID: %d, ItemQuant: %d",dwCovenantGUID,dwObjectGUID,dwMaintainType,pcObj->GetItemModelID(),pcObj->m_dwQuantity);

										}
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

										//If there are items required to maintain the dwelling
										if (fetchSuccess)
										{
											sprintf( szCommand, "SELECT Name FROM avatar WHERE AvatarGUID = %08x;",OwnerID);
											retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
											retcode = SQLExecute( cDatabase::m_hStmt );
										
											retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, szName, sizeof( szName ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)

											//Return SQL_SUCCESS if there is a player that corresponds to owner of the fetched house
											if (SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS)
											{
												std::string		str1 = szName;
  												strPlayerName.assign(str1);
											}
											retcode = SQLCloseCursor( cDatabase::m_hStmt );					CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
											retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

											if (dwMaintainRequired > dwMaintainPaid)
											{
												int remainingMaintenance = 0;
												remainingMaintenance = dwMaintainRequired - dwMaintainPaid;
												
												if (remainingMaintenance < pcObj->m_dwQuantity)
												{
													dwMaintainPaid += remainingMaintenance;
													pcObj->m_dwQuantity -= remainingMaintenance;
												} else {
													dwMaintainPaid += pcObj->m_dwQuantity;
													pcObj->m_dwQuantity = 0;
												}
												
												sprintf( szCommand, "UPDATE houses_maintenance SET Paid = %d WHERE HouseID = %d AND ItemLinker = %d;",dwMaintainPaid,dwHouseID,pcObj->GetItemModelID());
												retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLExecute( cDatabase::m_hStmt );
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												
												/* F7B0:0x0228 Message */
												cMessage cmMaintenance;
												cmMaintenance << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0228L;

												sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLFetch( cDatabase::m_hStmt );
												if( retcode == SQL_NO_DATA )
													dwMaintenanceCount = 0;
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												cmMaintenance << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling
												
												//String maintainName;
												//String maintainPluralName;	
										
												sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );														
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

													cmMaintenance	<< dwMaintainRequired			// quantity required
																	<< dwMaintainPaid				// quantity paid
																	<< DWORD(pcModel->m_wModel)		// item's object type
																	<< pcModel->m_strName.c_str()	// name of this item
																	<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
												}
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
												
												AddPacket( WORLD_SERVER, cmMaintenance, 4 );

												/* F7B0:0x021D Message */
												cMessage cmDisplayPanel;
												cmDisplayPanel << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x021DL 
												<< dwCovenantGUID;			// Covenant Crystal's GUID

												DWORD ownerType =		0x00000001;	// Self = 0x00000001; Allegiance = 0x00000003
												DWORD levelReq =		0x00000001;	// Apartment, Cottage = 0x00000014; Villa = 0x00000023; Mansion = 0x00000032
												DWORD unknown2 =		0xFFFFFFFF;
												DWORD rankReq =			0xFFFFFFFF;	// Apartment, Cottage, Villa = 0xFFFFFFFF; Mansion = 0x00000006
												DWORD unknown3 =		0xFFFFFFFF;
												DWORD unknown4 =		0x00000000;
												DWORD unknown5 =		0x00000001;
												DWORD ownerName=		0x00000000;
												DWORD purchaseCount =	0x00000001;	// Apartment = 2; Cottage, Villa = 3; Mansion = 6;
												cmDisplayPanel	<< dwHouseID		// Dwelling ID
																<< OwnerID			// Dwelling Owner ID (NULL == 0x00000000)
																<< ownerType
																<< levelReq			// level requirement to purchase this dwelling (-1 if no requirement)
																<< unknown2
																<< rankReq			// rank requirement to purchase this dwelling (-1 if no requirement)
																<< unknown3
																<< unknown4
												//				<< unknown5
																<< strPlayerName.c_str();	// name of the current owner (NULL == 0x00000000)

												sprintf( szCommand, "SELECT COUNT(ID) FROM houses_purchase WHERE HouseID = %d;",dwHouseID );
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwPurchaseCount, sizeof( dwPurchaseCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLFetch( cDatabase::m_hStmt );
												if( retcode == SQL_NO_DATA )
													dwPurchaseCount = 0;
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												cmDisplayPanel << dwPurchaseCount;		// the number of items required to purchase this dwelling

												//String buyName;
												//String buyPluralName;

												sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_purchase WHERE HouseID = %d;",dwHouseID );														
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwBuyType, sizeof( dwBuyType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
												retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwBuyRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwBuyPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

												for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
												{
													if( retcode == SQL_NO_DATA )
													{
														dwBuyRequired = 1;
														dwBuyPaid = 0;
														dwBuyType = 12;
													}
													cItemModels *pcModel = cItemModels::FindModel(dwBuyType);
													std::string		strPluralName = pcModel->m_strName.c_str();
  													strPluralName.assign(strPluralName + "s");

													cmDisplayPanel	<< dwBuyRequired				// quantity required
																	<< dwBuyPaid					// quantity paid
																	<< DWORD(pcModel->m_wModel)		// item's object type
																	<< pcModel->m_strName.c_str()	// name of this item
																	<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
												}
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

												sprintf( szCommand, "SELECT COUNT(ID) FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintenanceCount, sizeof( dwMaintenanceCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLFetch( cDatabase::m_hStmt );
												if( retcode == SQL_NO_DATA )
													dwMaintenanceCount = 0;
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )	
												cmDisplayPanel << dwMaintenanceCount;	// the number of items required to pay the maintenance cost for this dwelling
													
												//String maintainName;
												//String maintainPluralName;	
												
												sprintf( szCommand, "SELECT ItemLinker,Required,Paid FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );														
												retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
												retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMaintainType, sizeof( dwMaintainType ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
												retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwMaintainRequired, sizeof( dwMaintainRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_ULONG, &dwMaintainPaid, sizeof( dwMaintainPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)

												for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
												{
													if( retcode == SQL_NO_DATA )
													{
														dwMaintainRequired = 1;
														dwMaintainPaid = 0;
														dwMaintainType = 12;
													}
													cItemModels *pcModel = cItemModels::FindModel(dwMaintainType);
													std::string		strPluralName = pcModel->m_strName.c_str();
  													strPluralName.assign(strPluralName + "s");

													cmDisplayPanel	<< dwMaintainRequired			// quantity required
																	<< dwMaintainPaid				// quantity paid
																	<< DWORD(pcModel->m_wModel)		// item's object type
																	<< pcModel->m_strName.c_str()	// name of this item
																	<< strPluralName.c_str();		// plural name of this item (if not specified, use <name> followed by 's' or 'es')
												}
												retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
												retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

												AddPacket(WORLD_SERVER,cmDisplayPanel,4);

												//Alter avatar inventory accordingly
												if (pcObj->m_dwQuantity <= 0)
												{
													// Destroy object
													cMessage cmDestroyObject;
													cmDestroyObject << 0x0024L << dwObjectGUID;
													AddPacket( WORLD_SERVER, cmDestroyObject, 4 );
													m_pcAvatar->RemoveInventory( pcObj );
												} else {
													cMasterServer::ServerMessage(ColorGreen,this,"ItemModelID: %d, ItemQuant: %d",pcObj->GetItemModelID(),pcObj->m_dwQuantity);
													//Adjust stack size
													cMessage cmAdjustStack;
													//cItemModels *pcModel = cItemModels::FindModel( pcObj->m_dwItemModelID );
													cmAdjustStack	<< 0x0197L
																	<< BYTE(0x00)					//BYTE sequence -- Seems to be a sequence number of some sort 
																	<< DWORD(pcObj->GetGUID( ))		//Object item -- Item getting its stack adjusted.
																	<< DWORD(pcObj->m_dwQuantity)	//DWORD count -- New number of items in the stack. 
																	<< DWORD(pcObj->m_dwQuantity);	//DWORD (pcObj->m_dwQuantity * pcModel->m_dwValue); //DWORD value -- New value for the item.
													AddPacket( WORLD_SERVER, cmAdjustStack, 4 );
												}
											}
										}
									}

									BOOLEAN maintenancePaid = true;
									DWORD	dwRequired;
									DWORD	dwPaid;
									sprintf( szCommand, "SELECT Required,Paid FROM houses_maintenance WHERE HouseID = %d;",dwHouseID );														
									retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwRequired, sizeof( dwBuyRequired ), NULL );		CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_ULONG, &dwPaid, sizeof( dwBuyPaid ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
									{
										if (dwRequired > dwPaid) maintenancePaid = false; // determine whether all items are now paid
									}
									retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
									if (maintenancePaid)
									{
										cMasterServer::ServerMessage(ColorGreen,this,"Maintenance paid.");
									} else {
										cMasterServer::ServerMessage(ColorGreen,this,"Maintenance partially paid.");
									}

									break;
								}
								
								case HOUSE_ABANDON:			//0x021F
								{
									//"You abandon your house!"
									AddPacket( WORLD_SERVER, m_pcAvatar->HouseAbandon( ++m_dwF7B0Sequence ), 4 );
									break;
								}
								
								case HOUSE_GUEST_ADD:		//0x0245
								{									
									WORD wLength;
									char strGuestName[50];
									CopyMemory( &wLength, &pbData[12], 2 );
									CopyMemory( strGuestName, &pbData[14], wLength );
									strGuestName[wLength] = '\0';
								/*
									//Reverse bits
									int i,j, c;
									for (i = 0, j = strlen(strName)-1; i<j; i++, j--) {
										c = strGuestName[i];
										strGuestName[i] = strGuestName[j];
										strGuestName[j] = c;
									}
									wLength = atoi(strGuestName);
								*/
									m_pcAvatar->HouseGuestAdd(strGuestName);
									break;
								}
								
								case HOUSE_GUEST_REM_NAME:	//0x0246 // by name
								{
									WORD	wLength;
									char	strGuestName[50];
									CopyMemory( &wLength, &pbData[12], 2 );
									CopyMemory( strGuestName, &pbData[14], wLength );
									strGuestName[wLength] = '\0';

									m_pcAvatar->HouseGuestRemoveName(strGuestName);
									break;
								}
								
								case HOUSE_OPEN_CLOSE:		//0x0247 //Open or Close
								{
									m_pcAvatar->HouseOpenClose( MType3 );
									break;
								}
								
								case HOUSE_STORAGE:			//0x0249 //Add or Remove by Name
								{
									WORD	wLength;
									char	strGuestName[50];
									DWORD	dwStorageSet;

									CopyMemory( &wLength, &pbData[12], 2 );
									CopyMemory( strGuestName, &pbData[14], wLength );
									strGuestName[wLength] = '\0';

									while ((wLength % 2 != 0) || (wLength % 4 == 0)) { wLength++; }	//account for string padding
									CopyMemory( &dwStorageSet, &pbData[14+wLength], 4 );

									m_pcAvatar->HouseStorage(strGuestName,dwStorageSet);
									break;
								}
								
								case HOUSE_BOOT_NAME:		//0x024A //Boot by Name
								{
									WORD	wLength;
									char	strGuestName[50];
									CopyMemory( &wLength, &pbData[12], 2 );
									CopyMemory( strGuestName, &pbData[14], wLength );
									strGuestName[wLength] = '\0';

									m_pcAvatar->HouseBootName(strGuestName);
									break;
								}
								
								case HOUSE_STORE_REM_ALL:	//0x024C //Remove all from storage
								{
									m_pcAvatar->HouseStorageRemoveAll( );
									break;
								}
								
								case HOUSE_GUEST_LIST:		//0x024D
								{	
									if (m_pcAvatar->m_wHouseID)
									{
										AddPacket (WORLD_SERVER, m_pcAvatar->HouseGuestList( ++m_dwF7B0Sequence ), 4);
									}
									break;
								}
								
								case SET_SPEAKER:			//0x0251
								{
									break;
								}
								
								case SHOW_SPEAKER:			//0x0252
								{
									break;
								}
								
								case CLEAR_SPEAKER:			//0x0253
								{
									break;
								}
								
								case SET_MOTD:				//0x0254
								{
									break;
								}
								
								case SHOW_MOTD:				//0x0255
								{
									break;
								}
								
								case CLEAR_MOTD:			//0x0256
								{
								 	break;
								}
								 
								case HOUSE_GUEST_REM_ALL:	//0x025E
								{
									char	szCommand[512];
									RETCODE	retcode;

									char	szPacket[60];

									if (m_pcAvatar->m_wHouseID)
									{
										sprintf( szCommand, "DELETE FROM houses_guest_lists WHERE HouseID = %d;",m_pcAvatar->m_wHouseID );
										retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );		CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
											
										sprintf (szPacket, "You have removed all the guests from your house.");
										cMasterServer::ServerMessage(ColorGreen,this,(char *)szPacket);
										retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )		
									}
									break;
								}
								
								case HOUSE_BOOT_ALL:		//0x025F
								{
									break;
								}
								
								case HOUSE_RECALL:			//0x0262
								{
									//If the client owns a house
									if (m_pcAvatar->m_wHouseID)
									{
										cMessage cmHRAnim = m_pcAvatar->HRAnimate();	//The House Recall animation
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmHRAnim, 3 );

										cMessage cmHRMessage = m_pcAvatar->HRMessage();	//The House Recall message
										cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location,cmHRMessage, 4 );
										m_pcAvatar->m_dwPingCount = 0;
										m_pcAvatar->m_wHouseRecall = 1;
									}
									break;
								}
								
								case GET_OBJECT_MANA:		//0x0263 //Buggy...
								{
									DWORD dwObjectGUID = *( DWORD * )&pbData[12];
									if ( !dwObjectGUID )
										break;	

									cObject *pcObj = m_pcAvatar->FindInventory( dwObjectGUID );
									if( !pcObj )
										if( !( pcObj = cWorldManager::FindObject( dwObjectGUID ) ) )
											break; 
									
									cMessage cmUpdateMana;
									cmUpdateMana << 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence << 0x0264L << dwObjectGUID << 0L << 0L;
									AddPacket( WORLD_SERVER, cmUpdateMana, 4 );

									break;
								}
								
								case HOUSE_HOOKS:			//0x0266 //On or Off
								{
									char	szCommand[512];
									RETCODE	retcode;

									char	HookIDBuff[9];
									DWORD	HookID;
									WORD	wHasItem = 0;

									//If the client owns a house
									if (m_pcAvatar->m_wHouseID)
									{
										if (MType3 == 1)
										{
											sprintf( szCommand, "UPDATE houses_covenants SET HooksOn = %d WHERE HouseID = %d;",1,m_pcAvatar->m_wHouseID );
										} else {
											sprintf( szCommand, "UPDATE houses_covenants SET HooksOn = %d WHERE HouseID = %d;",0,m_pcAvatar->m_wHouseID );
										}
										retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
										retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_CLOSE );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )

										sprintf( szCommand, "SELECT GUID,HasItem FROM houses_hooks WHERE HouseID = %d;",m_pcAvatar->m_wHouseID );

										retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
										retcode = SQLExecute( cDatabase::m_hStmt );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )	
										retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, HookIDBuff, sizeof( HookIDBuff ), NULL );				CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
											
										for ( int i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
										{
											if (wHasItem != 1)
											{
												sscanf(HookIDBuff,"%08x",&HookID);

												cMessage cmHookVisible;
												cmHookVisible	<< 0xF74B 
																	<< HookID;
												if (MType3 == 1) {
													cmHookVisible	<< WORD(0x0014);
												} else {
													cmHookVisible	<< WORD(0x0034);
												}
												cmHookVisible	<<  WORD(0x0000)
																<< WORD(m_pcAvatar->m_wNumLogins)
																<< WORD(m_pcAvatar->m_wPortalCount++);
												AddPacket( WORLD_SERVER, cmHookVisible, 4 );

												cMessage cmHookBoolean;
												cmHookBoolean	<< 0x02D2L		//Set Object Boolean
																<< BYTE(0x04)	//BYTE sequence
																<< HookID		//Hook GUID
																<< DWORD(0x18L);//Hook Visibility

												//Boolean value - Boolean property value (0 = False, 1 = True)
												if (MType3 == 1) {
													cmHookBoolean	<< DWORD(0x0L);
												} else {
													cmHookBoolean	<< DWORD(0x1L);
												}
												AddPacket( WORLD_SERVER, cmHookBoolean, 4 );					
											}
											retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
											retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
										}
									}
									break;
								}
								
								case HOUSE_GUEST_ALLEG:		//0x0267 // Add or Remove
								{
									//"You have granted your monarchy access to your dwelling."
									//"The monarchy already has access to your dwelling."
									//"You have revoked access to your dwelling to your monarchy."
									//"The monarchy did not have access to your dwelling."
									//"You must be part of an allegiance to use this command."
									break;
								}
								
								case JOIN_CHESS_GAME:		//0x0269
								{
									break;
								}
								
								case LEAVE_CHESS_GAME:		//0x026A
								{
									break;
								}
								
								case MOVE_CHESS_PIECE:		//0x026B
								{
									break;
								}
								
								case OFFER_CHESS_DRAW:		//0x026E
								{
									break;
								}
								
								case HOUSE_AVAIL:			//0x0270
								{								
									char	szCommand[512];
									RETCODE	retcode;
									DWORD	dwCottageCount, dwVillaCount, dwMansionCount, dwApartmentCount;
									dwCottageCount = dwVillaCount = dwMansionCount = dwApartmentCount = 0;
											
									char	dwLandblockBuff[9];
									DWORD	dwLandblock;

									int		i;

									cMessage cmHouseAvail;
									cmHouseAvail	<< 0xF7B0L << m_pcAvatar->GetGUID( ) << ++m_dwF7B0Sequence
													<< 0x0271L;

									//Determine cottage availability
									sprintf( szCommand, "SELECT COUNT(ID) FROM houses_covenants WHERE HouseType=1 AND OwnerID = %d;",0 );
									retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwCottageCount, sizeof( dwCottageCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLFetch( cDatabase::m_hStmt );
											
									if( retcode == SQL_NO_DATA )
										dwCottageCount = 0;
									/*
									if (dwCovenantCount != 1)
									{
										sprintf (szPacket, "There are %d cottages available.",dwCovenantCount);
									} else {
										sprintf (szPacket, "There is 1 cottage available.");
									}
									cMasterServer::ServerMessage(ColorGreen,this,(char *)szPacket);
									*/
									cmHouseAvail << dwCottageCount;

									retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

									//sprintf( szCommand, "SELECT Landblock, Position_X, Position_Y FROM houses_covenants WHERE HouseType=1 AND OwnerID IS NULL;" );
									sprintf( szCommand, "SELECT Landblock FROM houses_covenants WHERE HouseType=1 AND OwnerID = %d;",0 );										
									retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLExecute( cDatabase::m_hStmt );
												
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, dwLandblockBuff, sizeof( dwLandblockBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
									//retcode = SQLBindCol( cDatabase::m_hStmt, 2, SQL_C_FLOAT, &flX, sizeof( float ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									//retcode = SQLBindCol( cDatabase::m_hStmt, 3, SQL_C_FLOAT, &flY, sizeof( float ), NULL );						CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
											
									for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
									{
										sscanf(dwLandblockBuff,"%08x",&dwLandblock);
										/*
										float lat, lng;
										//lat = ((((dwLandblock >> 16) & 0xFF) - 0x7F) * 192 + flY - 84) / 240;
										//lng = ((((dwLandblock >> 24) & 0xFF) - 0x7F) * 192 + flX - 84) / 240;
										lat = ((((((dwLandblock & 0x00FF0000) / 0x0010000) & 0xFF) + 1) * 8) + static_cast<float>(flY / 24) - 1027.5) / 10; 
										lng = ((((((dwLandblock & 0xFF000000) / 0x1000000) & 0xFF) + 1) * 8) + static_cast<float>(flX / 24) - 1027.5) / 10;

										char ns[10];
										char ew[10];
										if (lat > 0) { sprintf (ns, "%.2fN",lat); }
										else { sprintf (ns, "%.2fS",fabs(lat)); }
										if (lng > 0) {sprintf (ew, "%.2fE",lng); }
										else { sprintf (ew, "%.2fW",fabs(lng)); }
										
										sprintf (szPacket, "%s%s, %s",indent,ns,ew);
										cMasterServer::ServerMessage(ColorGreen,this,(char *)szPacket);
										*/
										cmHouseAvail << dwLandblock;
									}
									retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

									//Determine villa availability
									sprintf( szCommand, "SELECT COUNT(ID) FROM houses_covenants WHERE HouseType=2 AND OwnerID = %d;",0 );
									retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwVillaCount, sizeof( dwVillaCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLFetch( cDatabase::m_hStmt );
										
									if( retcode == SQL_NO_DATA )
										dwVillaCount = 0;

									cmHouseAvail << dwVillaCount;

									retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

									sprintf( szCommand, "SELECT Landblock FROM houses_covenants WHERE HouseType=2 AND OwnerID = %d;",0 );										
									retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLExecute( cDatabase::m_hStmt );

									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, dwLandblockBuff, sizeof( dwLandblockBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
											
									for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i ) {
										sscanf(dwLandblockBuff,"%08x",&dwLandblock);
										cmHouseAvail << dwLandblock;
									}
									retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );

									//Determine mansion availability
									sprintf( szCommand, "SELECT COUNT(ID) FROM houses_covenants WHERE HouseType=3 AND OwnerID = %d;",0 );
									retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwMansionCount, sizeof( dwMansionCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLFetch( cDatabase::m_hStmt );
										
									if( retcode == SQL_NO_DATA )
										dwMansionCount = 0;

									cmHouseAvail << dwMansionCount;

									retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )

									sprintf( szCommand, "SELECT Landblock FROM houses_covenants WHERE HouseType=3 AND OwnerID = %d;",0 );										
									retcode = SQLPrepare( cDatabase::m_hStmt, (unsigned char *)szCommand, SQL_NTS );								CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLExecute( cDatabase::m_hStmt );

									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_CHAR, dwLandblockBuff, sizeof( dwLandblockBuff ), NULL );	CHECKRETURN(1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1)
											
									for ( i = 0; SQLFetch( cDatabase::m_hStmt ) == SQL_SUCCESS; ++i )
									{
										sscanf(dwLandblockBuff,"%08x",&dwLandblock);
										cmHouseAvail << dwLandblock;
									}
									retcode = SQLCloseCursor( cDatabase::m_hStmt ); CHECKRETURN(0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL)
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );
									
									//Determine apartment availability
									sprintf( szCommand, "SELECT COUNT(ID) FROM houses_covenants WHERE HouseType=4 AND OwnerID = %d;",0 );
									retcode = SQLPrepare( cDatabase::m_hStmt, (BYTE *)szCommand, SQL_NTS );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLExecute( cDatabase::m_hStmt );								CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLBindCol( cDatabase::m_hStmt, 1, SQL_C_ULONG, &dwApartmentCount, sizeof( dwApartmentCount ), NULL );	CHECKRETURN( 1, SQL_HANDLE_STMT, cDatabase::m_hStmt, 1 )
									retcode = SQLFetch( cDatabase::m_hStmt );
											
									if( retcode == SQL_NO_DATA )
										dwApartmentCount = 0;

									retcode = SQLCloseCursor( cDatabase::m_hStmt );				CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
									retcode = SQLFreeStmt( cDatabase::m_hStmt, SQL_UNBIND );	CHECKRETURN( 0, SQL_HANDLE_STMT, cDatabase::m_hStmt, NULL )
									
									cmHouseAvail	<< dwCottageCount
													<< dwVillaCount
													<< dwMansionCount
													<< dwApartmentCount
													<< DWORD(0L);
									AddPacket( WORLD_SERVER, cmHouseAvail, 4 );
									break;
								}
								
								case ANSWER_POPUP:			//0x0275 // Answers a yes/no pop-up question
								{
									DWORD type;
									DWORD confirmSeq;
									DWORD answer;

									CopyMemory( &type, &pbData[12], 4);
									CopyMemory( &confirmSeq, &pbData[16], 4);
									CopyMemory( &answer, &pbData[20], 4);
									
									cWorldManager::FindPendingConfirm(confirmSeq, answer);

									break;
								}
								
								case ALLEG_BOOT:			//0x0277 // Boot Member by Name
								{
									break;
								}
								
								case ALLEG_RECALL:			//0x0278 // Includes Mansion Recall
								{
									break;
								}
								
								case SUICIDE:				//0x0279 // (@die)
								{
									break;
								}
								
								case ALLEG_INFO:			//0x027B // Retrieve Info by Name
								{
									break;
								}
								
								case MARKETPLACE_RECALL:	//0x028D
								{
									cMessage cmMPAnim = m_pcAvatar->MPAnimate();	//The Marketplace Recall animation
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmMPAnim, 3 );

									cMessage cmMPMessage = m_pcAvatar->MPMessage();	//The Marketplace Recall animation
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location,cmMPMessage, 4 );
									m_pcAvatar->m_dwPingCount = 0;
									m_pcAvatar->m_wMarketplace = 1;
									break;
								}
									
								case PKLITE:				//0x028F // (@pklite)
								{
									cMessage cmLSAnim = m_pcAvatar->LSAnimate();
									cWorldManager::SendToAllInFocus( m_pcAvatar->m_Location, cmLSAnim, 3 );
									m_pcAvatar->m_dwPingCount = 0;
									m_pcAvatar->m_wPKlite = 1;
									break;
								}
								
								case PROMOTE_FELLOW_LEADER:	//0x0290
								{
									DWORD dwNewLeaderGUID;
									CopyMemory( &dwNewLeaderGUID, &pbData[12], 4 );

									m_pcAvatar->FellowshipLeader( dwNewLeaderGUID );
									break;
								}
								
								case OPEN_CLOSE_FELLOWSHIP:	//0x0291
								{
									DWORD dwOpen;
									CopyMemory( &dwOpen, &pbData[12], 4 );

									m_pcAvatar->OpenCloseFellowship( dwOpen );
									break;
								}
								
								//High-Priority Movement Packet
								case MOVEMENT_HIGH_PRIORITY: //0xF753:
								{
									m_pcAvatar->m_wLifeStone = 0;
									cLocation *pcLoc = (cLocation *)&pbData[12];
								
									/*
									#ifdef _DEBUG
										char szPacketA[120];
										sprintf( szPacketA, "Movement High: H1: %f U1: %f U2: %f H2: %f",pcLoc->m_flA,pcLoc->m_flB, pcLoc->m_flC,pcLoc->m_flW);
										cMasterServer::ServerMessage( ColorYellow2,this,(char *)szPacketA);
									 #endif
									*/

									cWorldManager::MoveAvatar( this, *pcLoc );
									break;	// case 0xF753
								}

								//Low-Priority Movement Packet
								case MOVEMENT_LOW_PRIORITY: //0xF61C:
								{
									m_pcAvatar->m_wLifeStone = 0;
									DWORD *pdwData = (DWORD *)pbData;
									if ( pdwData[3] == 0 || pdwData[3] == 1 )
									{
										cWorldManager::MoveAvatar( this, m_pcAvatar->m_Location, 1 );
										break;	// case 0xF61C
									}

									cLocation *pcLoc;
									DWORD dwSkip = 0;
									DWORD dwHex3;

									for ( dwHex3 = pdwData[3]; dwHex3 > 0; dwHex3 >>= 1 )
										if ( dwHex3 & 1 )
											++dwSkip;

									//Flags
									DWORD dwSpeedFlag = 0, dwTurnFlag = 0, dwSlideFlag = 0; 
									DWORD dwMoveFlag = 0, dwEmoteFlag = 0;

									DWORD *pdwDataAt = &pdwData[4];

									WORD wAnim			= 0;
									//Cubem0j0:  Note: this used to be 1.0
									float flPlaySpeed	= 1.0f;

									if ( pdwData[3] & 0x00000001 )
									{	//Normal Speed, Unset = Walking
										dwSpeedFlag = *pdwDataAt;
										pdwDataAt += 1;
									}

									if ( pdwData[3] & 0x00000004 )
									{	//Moving
										DWORD dwMoveFlag = *pdwDataAt;
										pdwDataAt += 1;

										if ( dwMoveFlag == 0x45000005 )
										{
											//Cubem0j0: Test to see if we can run faster!
											flPlaySpeed = 1.0f;
											if ( dwSpeedFlag == 0 )
												wAnim = 0x05;
											else if ( dwSpeedFlag == 2 )
												wAnim = 0x07;
										}
										else if ( dwMoveFlag == 0x45000006 )
										{
											flPlaySpeed = -1.0f;
											wAnim = 0x05;
										}
										else if ( (dwMoveFlag & 0xFF000000) == 0x41000000 )
											wAnim = (WORD)(dwMoveFlag & 0xFF);
										else if ( (dwMoveFlag & 0xFF000000) == 0x43000000 )
											wAnim = (WORD)(dwMoveFlag & 0xFF);
									}

									if ( pdwData[3] & 0x00000020 )
									{	//Sliding
										DWORD dwSlideFlag = *pdwDataAt;
										pdwDataAt += 1;

										if ( dwSlideFlag == 0x6500000F )
										{
											flPlaySpeed = 1.0f;
											wAnim = 0x0F;
										}
										else if ( dwSlideFlag == 0x65000010 )
										{
											flPlaySpeed = -1.0f;
											wAnim = 0x0F;
										}
									}

									if ( pdwData[3] & 0x00000100 )
									{	//Turning
										DWORD dwTurnFlag = *pdwDataAt;
										pdwDataAt += 1;
													
										if ( dwTurnFlag == 0x6500000D )			//Right
										{
											flPlaySpeed = 1.0f;
											wAnim = 0x0D;
										}
										else if ( dwTurnFlag == 0x6500000E )	//Left
										{
											flPlaySpeed = -1.0f;
											wAnim = 0x0D;
										}
									}

									if ( pdwData[3] & 0x0000F800 )
									{	//Emote Backlog
										DWORD dwEmoteFlag = *pdwDataAt;
										DWORD dwNumEmotes = (pdwData[3] & 0x0000F800) >> 11;

										pdwDataAt += (dwNumEmotes * 2);

										dwSkip += (dwNumEmotes * 2) - 1;
													
										if ( dwNumEmotes > 2 )	//For some reason, on 3+ emotes, it has another DWORD at the end
											++dwSkip;

										//Just play the first one
										wAnim = (WORD)(dwEmoteFlag & 0xFF);

										if ( dwNumEmotes > 1 )
										{
											//Lots of emotes...  Learn how to chain later! =]
										}
										cWorldManager::MoveAvatar( this, m_pcAvatar->m_Location, wAnim );
										break;	// case 0xF61C
									}

									pcLoc = (cLocation *)&pdwData[4 + dwSkip];
									/*
									#ifdef _DEBUG
										char szPacketA[120];
										sprintf( szPacketA, "Movement Low: H1: %f U1: %f U2: %f H2: %f",pcLoc->m_flA,pcLoc->m_flB, pcLoc->m_flC,pcLoc->m_flW);
										cMasterServer::ServerMessage( ColorYellow2,this,(char *)szPacketA);
									 #endif
									*/

									cWorldManager::MoveAvatar( this, *pcLoc, wAnim, flPlaySpeed );
									break;	// case 0xF61C
								} 

								case JUMP:
								{
									char szPacket[60];
									sprintf( szPacket, "F7B1 Packet: %08x",MType2);
									cMasterServer::ServerMessage( ColorYellow,this,(char *)szPacket);
									break;
								}


								default:
								{
                                 #ifdef _DEBUG
									char szPacket[60];
									sprintf( szPacket, "Unknown F7B1 Packet: %08x",MType2);
									cMasterServer::ServerMessage( ColorYellow,this,(char *)szPacket);
                                 #endif
								}
							}	//end switch ( MType2 )						
						break;
						}	//end case 0xF7B1
						
						default:
						{
						 #ifdef _DEBUG
							char szPacketA[60];
							sprintf( szPacketA, "Unknown World Packet: %08x",MType1);
							cMasterServer::ServerMessage( ColorYellow2,this,(char *)szPacketA);
						 #endif
						}
						
					}	//end switch ( MType1 )
					pbData += FH.m_wFragmentLength - 16;

				}	//end while loop
			}	//end if conditonal		
			break;	//case 4
		}
		case 5:	//Logout
		{
			cWorldManager::RemoveClient( this );
			m_bWorldState	= 1;
			m_bCharState	= 8;
			m_dwF7B0Sequence = 0;
		
			break;	//case 5
		}
	}	//end switch ( m_bWorldState )
}