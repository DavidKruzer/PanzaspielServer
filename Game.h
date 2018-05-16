/*
 * Game.h
 *
 *  Created on: 07.01.2018
 *      Author: david
 */

#ifndef GAME_H_
#define GAME_H_

#define maxProjectiles 100
#define projectileLifetime 10
#define maxHitDistanceSquared 2500
#define maxHealth 6

#include "NetworkClient.h"
#include "Leaderboard.hpp"


#include <math.h>

inline float to_rad(float degrees) {
	return degrees * M_PI / 180.0;
}


struct ProjectileCalcInformation
{
	ProjectileInformation projectileInformation;
	float lifetime;
	float direction[2];
};

class Game {
private:
	NetworkClient** players;
	unsigned int playerAmount;
	struct PanzerInformation* onlinePanzer;
	int* panzerHealth;
	struct ProjectileInformation* newProjectiles;
	struct ProjectileCalcInformation* allProjectiles;
    unsigned int currentProjectile;
    const float projectileSpeed = 700.0f;
    enum GameState gameState;
    int winner; //-1 when no one, 0 ... for Player 0...

	void explodeProjectile(unsigned int);

public:
	bool update(float);
	void finished(void);
	void freeMemory(void);
	void newGame(int, NetworkClient**);

	virtual ~Game();
};

#endif /* GAME_H_ */
