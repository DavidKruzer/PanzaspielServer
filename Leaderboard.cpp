/*
 * Leaderboard.cpp
 *
 *  Created on: Feb 15, 2018
 *      Author: david
 */




/*
 * Leaderboard.c
 *
 *  Created on: Feb 5, 2018
 *      Author: david
 */

#include "Leaderboard.hpp"

leaderboardItem_t* firstLeaderboardItem = NULL;

leaderboardItem_t* lastLeaderboardItem = NULL;

const int leaderboard_StartPoints = 1000;

int listAccessable = 1; //When list is accessed, set value to 0

void removeFromList(leaderboardItem_t*);
void addIntoList(leaderboardItem_t*, leaderboardItem_t*, leaderboardItem_t*);

void leaderboard_Initialise()
{
	firstLeaderboardItem = NULL;
	lastLeaderboardItem = NULL;
}

void leaderboard_PlayerChangePoints(char* playerName, int pointChange)
{
	//Make sure playerName terminates in time.

	//playerName[leaderboard_PlayerNameLength] = '\0';

	leaderboardItem_t* thisPlayer = firstLeaderboardItem;

	//Search for Player thisPlayer with same playerName in the List
	while(thisPlayer != NULL)
	{
		if(strcmp(playerName, thisPlayer->playerName) == 0)
		{
			//thisPlayer is player that wants to change points
			thisPlayer->points += pointChange;

			//Rearrange List, so player is at the right Position

			if(pointChange > 0)
			{
				//Player moves forwards in the list
				//Start search in front of old Position, then remove player from old location
				leaderboardItem_t* newPosition = thisPlayer->previous;
				removeFromList(thisPlayer);

				while(newPosition != NULL)
				{
					if(newPosition->points > thisPlayer->points)
					{
						//this player comes after newPosition
						addIntoList(thisPlayer, newPosition, newPosition->next);
						return;
					}
					newPosition = newPosition->previous;
				}

				//this player is now first
				addIntoList(thisPlayer, NULL, firstLeaderboardItem);
				return;
			}

			else if(pointChange < 0)
			{
				//Player moves backwards in the list
				//Start search right after old Position, then remove player from old position
				leaderboardItem_t* newPosition = thisPlayer->next;
				removeFromList(thisPlayer);

				while(newPosition != NULL)
				{
					if(newPosition->points < thisPlayer->points)
					{
						//this player comes before newPosition
						addIntoList(thisPlayer, newPosition->previous, newPosition);
						return;
					}
					newPosition = newPosition->next;
				}

				//This Player is now last
				addIntoList(thisPlayer, lastLeaderboardItem, NULL);
				return;
			}

			else
			{
				//No Change in Points, do nothing
				return;
			}
		}
		thisPlayer = thisPlayer->next;
	}


	//No existing player with his PlayerName, create new one in his place

	leaderboardItem_t* newPlayer = (leaderboardItem_t*) malloc(sizeof(leaderboardItem_t));
	memcpy(newPlayer->playerName, playerName, leaderboard_PlayerNameLength);
	newPlayer->points = leaderboard_StartPoints + pointChange;

	leaderboardItem_t* newPlayerPosition = firstLeaderboardItem;

	while(newPlayerPosition != NULL)
	{
		if(newPlayerPosition->points < newPlayer->points)
		{
			//Add new Player before newPlayerPosition
			addIntoList(newPlayer, newPlayerPosition->previous, newPlayerPosition);
			return;
		}
		newPlayerPosition = newPlayerPosition->next;
	}

	//Add new Player at the end of the list, after newPlayerPosition
	addIntoList(newPlayer, lastLeaderboardItem, NULL);
}


void addIntoList(leaderboardItem_t* newItem, leaderboardItem_t* previousItem, leaderboardItem_t* nextItem)
{
	newItem->previous = previousItem;
	newItem->next = nextItem;
	if(previousItem != NULL) previousItem->next = newItem;
	else
	{
		//New Item  is now First Item
		firstLeaderboardItem = newItem;
	}
	if(nextItem != NULL) nextItem->previous = newItem;
	else
	{
		lastLeaderboardItem = newItem;
	}
}

void removeFromList(leaderboardItem_t* item)
{
	if(item->previous != NULL)
	{
		item->previous->next = item->next;
	}
	else
	{
		//Item was first Item
		firstLeaderboardItem = item->next;
	}

	if(item->next != NULL)
	{
		item->next->previous = item->previous;
	}

	else
	{
		//Item was last Item
		lastLeaderboardItem = item->previous;
	}

	item->previous = NULL;
	item->next = NULL;
}

int leaderboard_Safe()
{
	FILE* fHandle = NULL;
	fHandle = fopen("./buecher_datenbank.csv", "w");

	if(fHandle == NULL)
	{
		printf("Problem opening file");
		return -1;
	}

	leaderboardItem_t* incriment = firstLeaderboardItem;
	while(incriment != NULL)
	{
		fprintf(fHandle, "%s,%d", incriment->playerName, incriment->points);
		incriment = incriment->next;
	}
	fclose(fHandle);

	return 0;
}

void leaderboard_Ausgabe(int forwards)
{
	if(forwards)
	{
		leaderboardItem_t* incriment = firstLeaderboardItem;
		printf("Leaderboard:\n--------------------------------------------\n");
		while(incriment != NULL)
		{
			printf("Player %s:\nPoints: %d\n----------------------------\n", incriment->playerName, incriment->points);
			incriment = incriment->next;
		}
	}

	else
	{
		leaderboardItem_t* incriment = lastLeaderboardItem;
		printf("Leaderboard:\n--------------------------------------------\n");
		while(incriment != NULL)
		{
			printf("Player %s:\nPoints: %d\n----------------------------\n", incriment->playerName, incriment->points);
			incriment = incriment->previous;
		}
	}
}


int leaderboard_CheckIntegrity()
{
	leaderboardItem_t* incriment = firstLeaderboardItem;
	if(incriment == NULL )
	{
		if(lastLeaderboardItem == NULL)
		{
			//List is empty, but correct
			return 0;
		}
		printf("FirstLeaderboardItem is NULL, lastLeaderboardItem not!\n");
		return -1;
	}

	int lastValue = incriment->points;

	while(incriment->next != NULL)
	{
		if(incriment->next->previous != incriment)
		{
			printf("List not linked properly. (Forwards check)\n");
			return -1;
		}
		incriment = incriment->next;

		if(lastValue < incriment->points)
		{
			printf("List not sorted properly. (Forwards check)\n");
			return -1;
		}

		lastValue = incriment->points;
	}
	//incriment should be last Item

	if(incriment != lastLeaderboardItem)
	{
		printf("LastLeaderboardItem is not correct!\n");
		return -1;
	}

	while(incriment->previous != NULL)
	{
		if(incriment->previous->next != incriment)
		{
			printf("List not linked properly. (Backwards check)");
		}
		incriment = incriment->previous;
		if(lastValue > incriment->points)
		{
			printf("List not sorted properly. %d > %d (Backwards check)\n", lastValue, incriment->points);
			return -1;
		}

		lastValue = incriment->points;
	}

	//incriment should be first Item

	if(incriment != firstLeaderboardItem)
	{
		printf("FirstLeaderboardItem is not correct!\n");
		return -1;
	}

	printf("List linked and sorted properly.\n");
	return 0;
}
