/*
 * PlayerName.cpp
 *
 *  Created on: Feb 15, 2018
 *      Author: david
 */

#include "PlayerName.hpp"

int playerName_verify(char* playerName)
{
	for(int i = 0; i < playerName_Length; i++)
		{
			if(playerName[i] == '\0')
			{
				if(i > 4)
				{
					return 0;
				}
				printf("Player Name %s too short.\n", playerName);
				return -1;
			}

			if(!((playerName[i] >= 'a' && playerName[i] <= 'z') || (playerName[i] >= 'A' && playerName[i] <= 'Z')))
			{
				printf("Player Name has invalid Characters.\n");
				return -1;
			}

		}
		printf("Player Name has no terminating Character.\n");
		return -1;
}
