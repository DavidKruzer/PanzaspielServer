/*
 * Game.cpp
 *
 *  Created on: 07.01.2018
 *      Author: david
 */

#include "Game.h"

void Game::newGame(int playerAmount, NetworkClient** networkClients)
{
	printf("Starting new Game\n");
	winner = -1;

	players = new NetworkClient*[playerAmount];
	this->playerAmount = playerAmount;

	onlinePanzer = new PanzerInformation[playerAmount];
	panzerHealth = new int[playerAmount];

	newProjectiles = new ProjectileInformation[playerAmount]; //Every player can max shoot one new Projectile per Update
	allProjectiles = new ProjectileCalcInformation[maxProjectiles];

	for(int i = 0; i < playerAmount; i ++)
	{
		newProjectiles[i].active = false;
		panzerHealth[i] = maxHealth;
	}

	currentProjectile = 0;

	gameState = PLAYING;

	for(int i = 0; i < playerAmount; i++)
	{
		printf("Linking Player %d with new Game\n", i);
		players[i]  = networkClients[i];
		players[i]->linkWithGame(&onlinePanzer[i], &newProjectiles[i], &panzerHealth[i], i);
		players[i]->sendGameInformation(playerAmount);
	}
}

bool Game::update(float passedTime)
{
	if(gameState == DISCONNECTED)
	{
		//Game is over
		return true;
	}

	if(gameState == WAITING)
	{
		return false;
	}

	if(gameState == FINISHED)
	{
		//Game is over
		return true;
	}

	int notDestroyedPanzer = -1; //stays -1 when every panzer destroyed, -2 when there are more then one still alive, 0, 1, 2,.. when only panzer 0, 1, 2,... survived

	for(unsigned int i = 0; i < playerAmount; i++)
	{
		if(panzerHealth[i] > 0)
		{
			if(notDestroyedPanzer == -1)
			{
				//This is first not destroyed panzer
				notDestroyedPanzer = i;
			}
			else if(notDestroyedPanzer >= 0)
			{
				//More then 1 not destroyed
				notDestroyedPanzer = -2;
			}
		}

		players[i]->sendOtherPanzer(onlinePanzer, panzerHealth, playerAmount);
		//Player shot new Projectile
		if(newProjectiles[i].active)
		{
			if(currentProjectile >= maxProjectiles)
			{
				currentProjectile = 0;
			}

			memcpy(&allProjectiles[currentProjectile].projectileInformation, &newProjectiles[i], sizeof(struct ProjectileInformation));
			allProjectiles[currentProjectile].lifetime = 0;
			allProjectiles[currentProjectile].direction[0] = -cos(to_rad(allProjectiles[currentProjectile].projectileInformation.rotation));
			allProjectiles[currentProjectile].direction[1] = sin(to_rad(allProjectiles[currentProjectile].projectileInformation.rotation));

			for(unsigned int i2 = 0; i2 < playerAmount; i2++)
			{
				//Send every Player the new Projectile
				players[i2]->sendNewProjectile(&allProjectiles[currentProjectile].projectileInformation, currentProjectile);
			}

			currentProjectile++;

			newProjectiles[i].active = false;
		}
	}

	for(unsigned int i = 0; i < maxProjectiles; i++)
	{
		if(allProjectiles[i].projectileInformation.active)
		{
			if(allProjectiles[i].lifetime < projectileLifetime)
			{
				allProjectiles[i].lifetime += passedTime;
				//this projectile is active, perform move and hit detection
				allProjectiles[i].projectileInformation.location[0] += allProjectiles[i].direction[0] * passedTime * projectileSpeed;
				allProjectiles[i].projectileInformation.location[1] += allProjectiles[i].direction[1] * passedTime * projectileSpeed;

				//Hit Detection for projectile i, with every player i2
				for(unsigned int i2 = 0; i2 < playerAmount; i2++)
				{
					//Projectile doesn't effect Panzer that shot it
					if(i2 != allProjectiles[i].projectileInformation.parent)
					{
						int distanceX = allProjectiles[i].projectileInformation.location[0] - onlinePanzer[i2].location[0];
						int distanceY = allProjectiles[i].projectileInformation.location[1] - onlinePanzer[i2].location[1];

						if((distanceX * distanceX + distanceY * distanceY) < maxHitDistanceSquared)
						{
							//Projectile i hit player i2
							panzerHealth[i2]--;
							explodeProjectile(i);
						}
					}
				}
			}
			else
			{
				explodeProjectile(i);
			}
		}
	}

	for(unsigned int i = 0; i < playerAmount; i++)
	{
		players[i]->sendBuffer();
	}

	if(notDestroyedPanzer >= 0)
	{
		//only one panzer survived
		winner = notDestroyedPanzer;
		finished();
		return true;
	}
	else if(notDestroyedPanzer == -1)
	{
		//no panzer survived
		winner = -1;
		finished();
		return true;
	}
	return false;
}

void Game::finished()
{
	GameFinishInformation gameFinishInformation;
	if(winner >= 0)
	{
		//Player winner won
		printf("Player %s won!\n", players[winner]->playerName);
		gameFinishInformation.type = 2;
		gameFinishInformation.player = winner;
		memcpy(gameFinishInformation.playerName, players[winner]->playerName, sizeof(gameFinishInformation.playerName));
	}
	if(winner == -1)
	{
		printf("Multiple Players won!\n");
		gameFinishInformation.type = 1;
	}
	for(unsigned int i = 0; i < playerAmount; i++)
	{
		if(i == winner) leaderboard_PlayerChangePoints(players[i]->playerName, 100);
		else leaderboard_PlayerChangePoints(players[i]->playerName, -100);
		players[i]->sendGameFinishInformation(&gameFinishInformation);
		players[i]->seperateFromGame();
		players[i] = NULL;
	}
	gameState = FINISHED;
	leaderboard_Ausgabe(1);
	freeMemory();
}

void Game::explodeProjectile(unsigned int index)
{
	allProjectiles[index].projectileInformation.active = false;
	for(unsigned int i = 0; i < playerAmount; i++)
	{
		//Send every Player the Projectile which is now inactive
		players[i]->sendNewProjectile(&allProjectiles[index].projectileInformation, index);
	}
}

Game::~Game() {
	freeMemory();
}

void Game::freeMemory()
{
	freeMemoryDef(players);
	freeMemoryDef(onlinePanzer);
	freeMemoryDef(panzerHealth);
	freeMemoryDef(newProjectiles);
	freeMemoryDef(allProjectiles);
}

