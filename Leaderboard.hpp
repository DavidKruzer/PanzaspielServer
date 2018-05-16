/*
 * Leaderboard.hpp
 *
 *  Created on: Feb 15, 2018
 *      Author: david
 */

#ifndef LEADERBOARD_HPP_
#define LEADERBOARD_HPP_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define leaderboard_PlayerNameLength 20

struct leaderboard_item_t {
	char playerName[leaderboard_PlayerNameLength];
	int points;

	struct leaderboard_item_t* previous;
	struct leaderboard_item_t* next;
};

typedef struct leaderboard_item_t leaderboardItem_t;


void leaderboard_Initialise();

void leaderboard_PlayerChangePoints(char*, int);

int leaderboard_safe(void);

void leaderboard_Ausgabe(int);

int leaderboard_CheckIntegrity();

#endif /* LEADERBOARD_HPP_ */
