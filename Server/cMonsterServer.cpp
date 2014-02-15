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
 *	@file cMonsterServer.cpp
 *	Implements functionality for the cMonsterServer class.
 *	Processes monster actions and sends the data to the client.
 *
 *	Author: G70mb2
 */

#include "cMonsterServer.h"
#include "WorldManager.h"
#include "MasterServer.h"
#include <algorithm>

DWORD	cMonsterServer::m_dwStatus;
time_t	cMonsterServer::longtime;
time_t	cMonsterServer::NextActionTime;

/**
 *	Processes monster actions.
 *
 *	This member function is called when a monster is set to perform a given action.
 *	It processess the actual packets that will be delivered based upon the input values. @see SimpleAI
 *	Inputs determine whether the monster state: idle, under attack, move to attacker, 
 *	there is a user within range, attack users, break off attack, change combat mode.
 *
 *	@param dwMonGUID - The GUID of the monstser to perform an action
 *	@param dwEvent - A numerical value represention the monster's current state.
 *	@param dwEngaged - 
 *	@param dwTarget - The GUID of the target for the monster to attack.
 */
void cMonsterServer::ProcessAction( DWORD dwMonGUID, DWORD dwEvent, DWORD dwEngaged, DWORD dwTarget )
{
	cObject *pcMonster = cWorldManager::FindObject( dwMonGUID );
	
	if(pcMonster) // If not a valid Monster Object, do nothing
	{
		cModels* pcModel = cModels::FindModel( pcMonster->GetMonsterModelID() );
		int iRand = rand();
		int iRand2 = rand();
		iRand = iRand + iRand2;
		iRand = (iRand > 6400) ? iRand % 6400 : iRand;
		iRand = (iRand < 1600) ? 1600 + iRand : iRand;

		if( pcModel )
		{
			switch( dwEvent )
			{
				case 0x0: // Idle 
					{
						WORD wAnimation;

							int IdleAnim = iRand/1000;

							switch( IdleAnim )
							{
							case 0:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[0];
									pcMonster->m_bIdleAnim++;
									break;
								}
								case 1:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[1];//0x53L;
									pcMonster->m_bIdleAnim++;
									break;
								}
								case 2:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[2];//0x52L;
									pcMonster->m_bIdleAnim++;
									break;
								}
								case 3:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[3];//0x50L;
									pcMonster->m_bIdleAnim++;
									break;
								}
								case 4:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[4];//0x53L;
									pcMonster->m_bIdleAnim++;
									break;
								}
								case 5:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[5];//0x52L;
									pcMonster->m_bIdleAnim = 0;
									break;
								}
							}
							
						//cMessage cMonAnim = pcMonster->Animation( wAnimation, 1.0f );
						cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->Animation( wAnimation, 1.0f ), 3 );
						
						break;
					}
				case 0x1: // Under attack
					{
						float flHeading, flDistance;
						
						cClient* pcTargetObj = cClient::FindClient( dwTarget );

						if( pcTargetObj)
						{
							flHeading = pcMonster->GetHeadingTarget( pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );
							cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->TurnToTarget(flHeading,dwTarget ), 3 );
						}

						flDistance = pcMonster->GetRange( pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );

						if ( flDistance < 0.8f)
						{
							SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 5, 1 );
							
						}
						else
						{
							SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 2, 1 );	
						}
						break;
					}
				case 0x2: // Move to attacker
					{
						float flDistance;
						cClient* pcTargetObj = cClient::FindClient( dwTarget );
						cMonster* pcMon = (cMonster *) pcMonster;
						pcMon->m_fHasTarget = false;
						if( pcTargetObj)
						{
							pcMon->m_fHasTarget = true;
							pcMon->m_dwTargetGUID = dwTarget;
							SimpleAI::SetMoving( pcMonster->GetGUID() );
							flDistance = pcMonster->GetRange(pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );

							if ( flDistance < 1.0f )
							{
								//cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->MoveTarget( pcTargetObj ), 3 );
								SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 5, 1 );
								//pcMonster->SetLocation(&pcTargetObj->m_pcAvatar->m_Location);
								
							}
							else
							{
								// Break off the chase if target is too far away
								if(flDistance > 3)//pcMonster->m_dwChase )
								{
									SimpleAI::SetAttackComplete( dwMonGUID );
									SimpleAI::SetAction( dwMonGUID, 3, 0);
									
								}
								else
								{
									SimpleAI::SetAttackEvent( dwMonGUID,dwTarget, 2, 3 );
									cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->MoveToTarget( pcTargetObj ), 3 );
									cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->SetPosition( ), 3 );
								}
							}
						}
						else
						{
							SimpleAI::SetAction( dwMonGUID, 0, 2);
						}

						break;
					}
				case 0x3: // Return to spawn point
					{
						float flDistance;

						flDistance = pcMonster->GetRange( pcMonster->m_SpawnLoc.m_dwLandBlock, pcMonster->m_SpawnLoc.m_flX, pcMonster->m_SpawnLoc.m_flY, pcMonster->m_SpawnLoc.m_flZ );

						if(( flDistance > 0.4f )&&( flDistance < pcMonster->m_dwChase))
						{
							cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->ReturnToSpawn( ), 3 );
							cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->SetPosition( ), 3 );
							SimpleAI::SetAction( dwMonGUID, 3, 4);
						}
						else if(flDistance > pcMonster->m_dwChase)
						{
							// Jump back to spawn point
							cWorldManager::MoveRemObject( pcMonster );
							pcMonster->SetLocation(&pcMonster->m_SpawnLoc);
							cWorldManager::MoveAddObject( pcMonster );
							cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->SetPosition( ), 3 );
							SimpleAI::SetAction( dwMonGUID, 0, 1 );

						}
						else
						{
							SimpleAI::SetAction( dwMonGUID, 0, 1 );
						}

						break;
					}
				case 0x4: // There is a user within range
					{
						SimpleAI::SetAction( dwMonGUID, 2, 3 );
						break;
					}
				case 0x5: // Attack user
					{
						DWORD dwDamageType = 0x4;
						float flDamageSlider = rand() % 2;
						float intRange;

						int AttackAnim = iRand/1000; 
						WORD wAttackAnim;

							switch( AttackAnim )
							{
							case 0:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[0];
									/*
									#ifdef _DEBUG
										char szPacketA[120];
										sprintf( szPacketA, "Attack Animation #: %d",AttackAnim);
										cMasterServer::ServerMessage( ColorBlue,NULL,(char *)szPacketA);
									#endif
									*/
									break;
								}
							case 1:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[1];
									break;
								}
							case 2:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[2];
									break;
								}
							case 3:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[3];
									break;
								}
							case 4:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[4];
									break;
								}
							case 5:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[5];
									break;
								}
							}
						
						cClient* pcTargetObj = cClient::FindClient( dwTarget );
						cMonster* pcMon = (cMonster *) pcMonster;
						pcMon->m_fHasTarget = false;
						if( pcTargetObj)
						{
							pcMon->m_fHasTarget = true;
							pcMon->m_dwTargetGUID = dwTarget;
							SimpleAI::SetMoving( pcMonster->GetGUID() );
							// Check range to target
							intRange = pcMonster->GetRange( pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );

							if( intRange < 1.5f)
							{
								//Take a swing
								cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->ChangeCombatMode( false ), 3 );
								cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->CombatAnimation( dwTarget,wAttackAnim ), 3 );

								WORD wTotalOpponentHealth = pcTargetObj->m_pcAvatar->GetTotalHealth();
								//Now perform a very basic combat calculation..nowhere near accurate.
								
								srand( timeGetTime( ) );
								float flMaxDamage = rand( )/27;
								
								//Cubem0j0:  Add strength to the monster's attack damage.
								//float mob_damage = pcModel->mob_strength / 2.5;
								DWORD dwTrueDamage = pcMonster->CalculateDamage(pcModel->mob_strength / pcTargetObj->m_pcAvatar->ALDamageReduction(pcTargetObj->m_pcAvatar->Armor_Level), 3.0f, 3.0f);
								double dSeverity = ( double ) wTotalOpponentHealth / dwTrueDamage;
								
								/* CubeM0j0:  Adjust for AL */
								UINT dwTrue2 = dwTrueDamage;
								//if(dwTrue2 < 0)
								//	dwTrue2 = 0;

								int iNewHealth;
								//CubeM0j0:  Check to see what the new numbers are.
								//#ifdef _DEBUG
								//	char szPacketA[100];
								//	sprintf( szPacketA, "dwTrueDamage: %d, dwTrue2: %d, Severity: %d",dwTrueDamage,dwTrue2,dSeverity);
								//	cMasterServer::ServerMessage( ColorGreen,NULL,(char *)szPacketA);
								//#endif

								BOOL fKilledPlayer = FALSE;
					
								//Following variable(s) only used if there is a kill
								cMessage cmAnim;
								cMessage cmHealthLoss = pcTargetObj->m_pcAvatar->DecrementHealth( dwTrue2, iNewHealth );

								pcTargetObj->AddPacket( WORLD_SERVER, cmHealthLoss, 4 );

								if( iNewHealth <= 0 ) 
								{
									fKilledPlayer = TRUE;

									//cMessage cmKill = pcTargetObj->m_pcAvatar->SetHealth( pcTargetObj->m_pcAvatar->GetTotalHealth( ) );
									
									//cMessage Anim = pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0 );
									cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0 ), 3 );
									
									//cMessage cAnim = pcTargetObj->m_pcAvatar->Animation( pcModel->m_cAnimations.m_wAttack[0], 2.0f );
									cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->Animation( pcModel->m_cAnimations.m_wAttack[0], 2.0f ), 3 );		
															
									cMasterServer::Corpse( pcTargetObj );

									//cMessage Alive = pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0 );
									cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE, 0 ), 3 );
									cWorldManager::TeleportAvatar( pcTargetObj, cMasterServer::m_StartingLoc );

									cMasterServer::ServerMessage( ColorGreen, pcTargetObj, "You have been restored to Health." );
									
								//	cMessage cmSetFlag;
								//	cmSetFlag << 0x022CL << BYTE(0) << pcTargetObj->m_pcAvatar->GetGUID() << 0x04L << 0;

								//	cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, cmSetFlag, 4 );
								//	pcTargetObj->m_pcAvatar->m_fIsPK = false;

									char szDeathNotice[200];
									wsprintf( szDeathNotice, "%s has killed %s", pcMonster->m_strName.c_str(), pcTargetObj->m_pcAvatar->m_strName.c_str( ) );
									
								//	cMessage cmDeathNotice;
								//	cmDeathNotice << 0xF62C << szDeathNotice << ColorGreen;
								//	cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, cmDeathNotice, 4 );

									pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->SetHealth( pcTargetObj->m_pcAvatar->GetTotalHealth( ) ), 4 );

									SimpleAI::SetAttackComplete( dwMonGUID );
									SimpleAI::SetTargetKilled( dwTarget );
								}
								else
								{
									cmAnim << pcMonster->CombatAnimation( dwTarget, wAttackAnim );

									//Cubem0j0: implementation of different hit locations.
									BYTE loc;
									int iloc = rand()%8;

									switch(iloc)
									{
									case 0:
										loc = 0x0;
										break;
									case 1:
										loc = 0x1;
										break;
									case 2:
										loc = 0x2;
										break;
									case 3:
										loc = 0x3;
										break;
									case 4:
										loc = 0x4;
										break;
									case 5:
										loc = 0x5;
										break;
									case 6:
										loc = 0x6;
										break;
									case 7:
										loc = 0x7;
										break;
									case 8:
										loc = 0x8;
										break;
									}
									
									//cMessage cmDamageRecieveMessage = pcTargetObj->m_pcAvatar->RecieveDamageMessage( ++pcTargetObj->m_dwF7B0Sequence, pcMonster->m_strName, dwDamageType, dSeverity, dwTrue2, loc);
									pcTargetObj->AddPacket( WORLD_SERVER, pcTargetObj->m_pcAvatar->RecieveDamageMessage( ++pcTargetObj->m_dwF7B0Sequence, pcMonster->m_strName, dwDamageType, dSeverity, dwTrue2, loc), 4 );
									//SimpleAI::SetAttackComplete( dwMonGUID );
									SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 5, 1 );
								}
							} // End range check
							else
							{
								SimpleAI::SetAttackEvent( dwMonGUID,dwTarget, 2, 0 );
							}

						}// End validaiton check
						break;
					}
				case 0x6: // Break off attack
					{
						SimpleAI::SetAction( dwMonGUID, 3, 7 );
						break;
					}
				case 0x7: // Change combat mode
					{
						if( pcMonster->m_fCombatMode == false )// Enter Melee Mode
						{
							//cMessage Anim = pcMonster->ChangeCombatMode( true  );
							cWorldManager::SendToAllInFocus( pcMonster->m_Location, pcMonster->ChangeCombatMode( true  ), 3 );
							pcMonster->m_fCombatMode = true;
						}
						else // Leave combat
						{
							//cMessage Anim = pcMonster->ChangeCombatMode( false );
							cWorldManager::SendToAllInFocus( pcMonster->m_Location, pcMonster->ChangeCombatMode( false ), 3 );
							pcMonster->m_fCombatMode = false;
							SimpleAI::SetAction( dwMonGUID, 0, 7 );
						}
						break;
					}
				} // End switch
		} // End pcModel check
	} 
	else
	{
		cMasterServer::ServerMessage( ColorBlue, NULL, "Monster Action Processing Error" );
	}
	// End check
}

//Karki

void cMonsterServer::ProcessPetAction( DWORD dwMonGUID, DWORD dwEvent, DWORD dwEngaged, DWORD dwTarget, DWORD dwOwner )
{
	//cMasterServer::ServerMessage( ColorBlue, NULL, "Pet Action Process Recieved" );
	cObject *pcPet = cWorldManager::FindObject( dwMonGUID );
	
	if(pcPet) // If not a valid Monster Object do nothing
	{
		cModels* pcModel = cModels::FindModel( pcPet->GetMonsterModelID() );
		int iRand = rand();
		int iRand2 = rand();
		iRand = iRand + iRand2;
		iRand = (iRand > 6400) ? iRand % 6400 : iRand;
		iRand = (iRand < 1600) ? 1600 + iRand : iRand;

		if( pcModel )
		{
			switch( dwEvent )
			{
				case 0x0: // Idle 
					{
						WORD wAnimation;
						float FollowRangeCheck;
						
						//Pet follow check
						cClient* pcPetTarget = cClient::FindClient( dwOwner );
						//cMasterServer::ServerMessage( ColorBlue, NULL, "Monster is Idle" );
						//cMasterServer::ServerMessage( ColorBlue, NULL, "%d is my Owner", dwOwner );
						//cMasterServer::ServerMessage( ColorBlue, NULL, "Owner = %d", pcPetTarget->FindClient( dwOwner) );
						if( pcPetTarget)
						{
							//cMasterServer::ServerMessage( ColorBlue, NULL, "Follow Checking" );
							// Check Range to Target
							FollowRangeCheck = pcPet->GetRange( pcPetTarget->m_pcAvatar->m_Location.m_dwLandBlock, pcPetTarget->m_pcAvatar->m_Location.m_flX, pcPetTarget->m_pcAvatar->m_Location.m_flY, pcPetTarget->m_pcAvatar->m_Location.m_flZ );
							//cMasterServer::ServerMessage( ColorBlue, NULL, "Range is %d", FollowRangeCheck );
							if( FollowRangeCheck >= .6)
							{
									cWorldManager::SendToAllWithin( 5, pcPet->m_Location, pcPet->MoveToTarget( pcPetTarget ), 3 );
									cWorldManager::SendToAllWithin( 10, pcPet->m_Location, pcPet->SetPosition( ), 3 );
									//cMasterServer::ServerMessage( ColorBlue, NULL, "Following" );

							}
						}

						int IdleAnim = iRand/1000;

						switch( IdleAnim )
						{
							case 0:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[0];
									pcPet->m_bIdleAnim++;
									break;
								}
								case 1:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[1];//0x53L;
									pcPet->m_bIdleAnim++;
									break;
								}
								case 2:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[2];//0x52L;
									pcPet->m_bIdleAnim++;
									break;
								}
								case 3:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[3];//0x50L;
									pcPet->m_bIdleAnim++;
									break;
								}
								case 4:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[4];//0x53L;
									pcPet->m_bIdleAnim++;
									break;
								}
								case 5:
								{
									wAnimation = pcModel->m_cAnimations.m_wIdle[5];//0x52L;
									pcPet->m_bIdleAnim = 0;
									break;
								}

						}
							
						cMessage cMonAnim = pcPet->Animation( wAnimation, 1.0f );
						cWorldManager::SendToAllWithin( 5, pcPet->m_Location, cMonAnim, 3 );
						
						break;
					}
					/*
				case 0x1: // Under attack
					{
						float flHeading, flDistance;
						
						cClient* pcTargetObj = cClient::FindClient( dwTarget );
						if( pcTargetObj)
						{
							flHeading = pcMonster->GetHeadingTarget( pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );
							cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->TurnToTarget(flHeading,dwTarget ), 3 );
						}

						flDistance = pcMonster->GetRange( pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );

						if ( flDistance < 0.8f)
						{
							SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 5, 1 );
							
						}
						else
						{
							SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 2, 1 );	
						}
						break;
					}
				case 0x2: // Move to attacker
					{
						float flDistance;
						cClient* pcTargetObj = cClient::FindClient( dwTarget );
						if( pcTargetObj)
						{
							flDistance = pcMonster->GetRange(pcTargetObj->m_pcAvatar->m_Location.m_dwLandBlock, pcTargetObj->m_pcAvatar->m_Location.m_flX, pcTargetObj->m_pcAvatar->m_Location.m_flY, pcTargetObj->m_pcAvatar->m_Location.m_flZ );

							if ( flDistance < 1.0f )
							{
								//cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->MoveTarget( pcTargetObj ), 3 );
								SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 5, 1 );
								pcMonster->SetLocation(&pcTargetObj->m_pcAvatar->m_Location);
								
							}
							else
							{
								// Break off the chase if the target is too far away
								if(flDistance > 3)//pcMonster->m_dwChase )
								{
									SimpleAI::SetAttackComplete( dwMonGUID );
									SimpleAI::SetAction( dwMonGUID, 3, 0);
									
								}
								else
								{
									SimpleAI::SetAttackEvent( dwMonGUID,dwTarget, 2, 3 );
									cWorldManager::SendToAllWithin( 5, pcMonster->m_Location, pcMonster->MoveToTarget( pcTargetObj ), 3 );
									cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->SetPosition( ), 3 );
								}
							}
						}
						else
						{
							SimpleAI::SetAction( dwMonGUID, 0, 2);
						}

						break;
					}
				case 0x3: // Return to spawn point
					{
						float flDistance;

						flDistance = pcMonster->GetRange( pcMonster->m_SpawnLoc.m_dwLandBlock, pcMonster->m_SpawnLoc.m_flX, pcMonster->m_SpawnLoc.m_flY, pcMonster->m_SpawnLoc.m_flZ );

						if(( flDistance > 0.4f )&&( flDistance < pcMonster->m_dwChase))
						{
							cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->ReturnToSpawn( ), 3 );
							cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->SetPosition( ), 3 );
							SimpleAI::SetAction( dwMonGUID, 3, 4);
						}
						else if(flDistance > pcMonster->m_dwChase)
						{
							// Jump back to Spawn Point
							cWorldManager::MoveRemObject( pcMonster );
							pcMonster->SetLocation(&pcMonster->m_SpawnLoc);
							cWorldManager::MoveAddObject( pcMonster );
							cWorldManager::SendToAllWithin( 10, pcMonster->m_Location, pcMonster->SetPosition( ), 3 );
							SimpleAI::SetAction( dwMonGUID, 0, 1 );

						}
						else
						{
							SimpleAI::SetAction( dwMonGUID, 0, 1 );
						}

						break;
					}
				case 0x4: // There is a user within range
					{
						SimpleAI::SetAction( dwMonGUID, 2, 3 );
						break;
					}
					*/
				case 0x5: // Attack user
					{
						DWORD dwDamageType = 0x4;
						float flDamageSlider = rand() % 2;
						//float intRange;

						int AttackAnim = iRand/1000; 
						WORD wAttackAnim;

							switch( AttackAnim )
							{
							case 0:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[0];
									break;
								}
							case 1:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[1];
									break;
								}
							case 2:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[2];
									break;
								}
							case 3:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[3];
									break;
								}
							case 4:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[4];
									break;
								}
							case 5:
								{
									wAttackAnim = pcModel->m_cAnimations.m_wAttack[5];
									break;
								}
							}
						/* Broken pet vombat vode
						cObject* pcTargetObj = cWorldManager::FindObject( dwTarget );
						if( pcTargetObj)
						{
							// Check range to target
							intRange = pcPet->GetRange( pcTarget->m_Location.m_dwLandBlock, pcTarget->m_Location.m_flX, pcTarget->m_Location.m_flY, pcTarget->m_Location.m_flZ );

							if( intRange < 1.5f)
							{
								//Take a swing
								cWorldManager::SendToAllWithin( 5, pcPet->m_Location, pcPet->ChangeCombatMode( false ), 3 );
								cWorldManager::SendToAllWithin( 5, pcPet->m_Location, pcPet->CombatAnimation( dwTarget,wAttackAnim ), 3 );

								WORD wTotalOpponentHealth = pcTarget->GetTotalHealth();
								//Now perform a very basic combat calculation..no where near accurate.
								
								srand( timeGetTime( ) );
								float flMaxDamage = rand( )/27;
								
								DWORD dwTrueDamage = pcPet->CalculateDamage(3.0f, 0.0f);
								double dSeverity = ( double ) wTotalOpponentHealth / dwTrueDamage;
								
								int iNewHealth;

								BOOL fKilledPlayer = FALSE;
					
								//Following variable(s) only used if there is a kill
								cMessage cmAnim;
								cMessage cmHealthLoss = pcTargetObj->m_pcAvatar->DecrementHealth( dwTrueDamage, iNewHealth );

								pcTargetObj->AddPacket( WORLD_SERVER, cmHealthLoss, 4 );

								if( iNewHealth <= 0 ) 
								{
									fKilledPlayer = TRUE;

									cMessage cmKill = pcTargetObj->m_pcAvatar->SetHealth( pcTargetObj->m_pcAvatar->GetTotalHealth( ) );
									
									cMessage Anim = pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE );
									cWorldManager::SendToAllWithin( 5, pcPet->m_Location, Anim, 3 );
									
									cMessage cAnim = pcTargetObj->m_pcAvatar->Animation( pcModel->m_cAnimations.m_wAttack[0], 2.0f );
									cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, cAnim, 3 );		
															
									cMasterServer::Corpse( pcTargetObj );

									cMessage Alive = pcTargetObj->m_pcAvatar->ChangeCombatMode( FALSE );
									cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, Alive, 3 );
									cWorldManager::TeleportAvatar( pcTargetObj, cMasterServer::m_StartingLoc );

									cMasterServer::ServerMessage( ColorGreen, pcTargetObj, "You have been restored to Health." );
									
									cMessage cmSetFlag;
									cmSetFlag << 0x022CL << BYTE(0) << pcTargetObj->m_pcAvatar->GetGUID() << 0x04L << 0;

									cWorldManager::SendToAllInFocus( pcTargetObj->m_pcAvatar->m_Location, cmSetFlag, 4 );
									pcTargetObj->m_pcAvatar->m_fIsPK = false;

									char szDeathNotice[200];
									wsprintf( szDeathNotice, "%s has killed %s", pcPet->m_strName.c_str(), pcTargetObj->m_pcAvatar->m_strName.c_str( ) );
									
									cMessage cmDeathNotice;
									cmDeathNotice << 0xF62C << szDeathNotice << ColorGreen;
									cWorldManager::SendToAllWithin( 5, pcPet->m_Location, cmDeathNotice, 4 );

									pcTargetObj->AddPacket( WORLD_SERVER, cmKill, 4 );

									SimpleAI::SetAttackComplete( dwMonGUID );
									SimpleAI::SetTargetKilled( dwTarget );
								}
								else
								{
									cmAnim << pcPet->CombatAnimation( dwTarget, wAttackAnim );

									cMessage cmDamageRecieveMessage = pcTargetObj->m_pcAvatar->RecieveDamageMessage( ++pcTargetObj->m_dwF7B0Sequence, pcPet->m_strName, dwDamageType, dSeverity, dwTrueDamage, 0x7 );
									pcTargetObj->AddPacket( WORLD_SERVER, cmDamageRecieveMessage, 4 );
									//SimpleAI::SetAttackComplete( dwMonGUID );
									SimpleAI::SetAttackEvent( dwMonGUID, dwTarget, 5, 1 );
								}
							} // End range check
							else
							{
								SimpleAI::SetAttackEvent( dwMonGUID,dwTarget, 2, 0 );
							}

						}// End validaiton check
						*/
						break;
					}
					/*
				case 0x6: // Break off attack
					{
						SimpleAI::SetAction( dwMonGUID, 3, 7 );
						break;
					}
				case 0x7: // Change combat mode
					{
						if( pcMonster->m_fCombatMode == false )// Enter Melee Mode
						{
							cMessage Anim = pcMonster->ChangeCombatMode( true  );
							cWorldManager::SendToAllInFocus( pcMonster->m_Location, Anim, 3 );
							pcMonster->m_fCombatMode = true;
						}
						else // Leave combat
						{
							cMessage Anim = pcMonster->ChangeCombatMode( false );
							cWorldManager::SendToAllInFocus( pcMonster->m_Location, Anim, 3 );
							pcMonster->m_fCombatMode = false;
							SimpleAI::SetAction( dwMonGUID, 0, 7 );
						}
						break;
					}
					*/
				} // End switch
		} // End pcModel check
	} 
	else
	{
		cMasterServer::ServerMessage( ColorBlue, NULL, "Monster Action Processing Error" );
	}
	// End check
}