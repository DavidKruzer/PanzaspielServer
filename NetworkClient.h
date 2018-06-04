/*
 * NetworkClient.h
 *
 *  Created on: 05.01.2018
 *      Author: david
 */

#ifndef NETWORKCLIENT_H_
#define NETWORKCLIENT_H_



#define freeMemoryDef(adress) if(adress) delete adress; adress = NULL;

#define closeSocketDef(socket) if(socket != -1){close(socket); printf("Client Socket %d closed.\n", socket); socket = -1;}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include "PlayerName.hpp"

#define bufferSize 100

enum GameState{WAITING, PLAYING, DISCONNECTED, FINISHED};

enum ClientState{CLIENT_EMPTY, CLIENT_CONNECTED,  CLIENT_VERIFIED, CLIENT_INQUEUE, CLIENT_INGAME, CLIENT_DISCONNECTED};

struct PanzerInformation
{
	unsigned int id;
	float location[2];
	float rotation;
	int movement;
	int health;
};

struct ProjectileInformation
{
	unsigned int id;
	unsigned int parent; //Panzer that shot the Projectile
	float location[2];
	float rotation;
};

struct GameInformation
{
	int myNumber; //Number of Player Information is sent to
	int playerAmount; //Total Amount of players
};

struct GameFinishInformation
{
	int type; //0: Game still goin, -1: Somebody disconnected, 1: Nobody won, 2: player won
	int player;
	char playerName[playerName_Length]; //Name of player who won
};

class NetworkClient {
private:
	int clientSocket;
	int clientNumber;
	pthread_t listenerThread;
	PanzerInformation* myPanzerInformation;
	ProjectileInformation* myNewProjectile;
	int* myPanzerHealth;

	int bufferPosition;
	char buffer[1500];

	static void* listenToClient(void*);
	void addInformationToBuffer(unsigned int, void*, unsigned int);
public:
	char playerName[20];
	ClientState clientState = CLIENT_EMPTY;
	void initNetworkClient(int);
	int linkWithGame(struct PanzerInformation*, struct ProjectileInformation*, int*, int);
	int seperateFromGame();
	int startListener(void);
	void sendGameInformation(int);
	void sendOtherPanzer(struct PanzerInformation*, int*, unsigned int);
	void sendNewProjectile(struct ProjectileInformation*);
    void sendRemoveProjectile(struct ProjectileInformation*);
	void sendGameFinishInformation(struct GameFinishInformation*);
	void sendBuffer();
	virtual ~NetworkClient();
};

#endif /* NETWORKCLIENT_H_ */
