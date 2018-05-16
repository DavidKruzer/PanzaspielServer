/*
 * Start.cpp
 *
 *  Created on: 05.01.2018
 *      Author: david
 */

#include "Game.h"

#define PORTNUM 2300

#define refreshRate 50

#define sendSleep 1000000 / refreshRate

#define maxGames 100

#define maxNetworkClients 200

void* updateGames(void*);
int startNewGame(void);
int getNewNetworkClient(void);
int getWaitingNetworkClients(NetworkClient**);

pthread_t updateThread;

Game games[maxGames];
bool activeGames[maxGames] = {false};

NetworkClient networkClients[maxNetworkClients];

int main(int argc, char *argv[])
{
	struct sockaddr_in dest; /* socket info about the machine connecting to us */
	struct sockaddr_in serv; /* socket info about our server */
	int mysocket;            /* socket used to listen for incoming connections */

	socklen_t socksize = sizeof(struct sockaddr_in);

	memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
	serv.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
	serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
	serv.sin_port = htons(PORTNUM);           /* set the server port number */

	printf("Get Socket\n");
	mysocket = socket(AF_INET, SOCK_STREAM, 0);


	printf("Bind Socket\n");
	/* bind serv information to mysocket */
	bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

	printf("Start listenings\n");

    //Setup thread to update Games
    if(pthread_create(&updateThread, NULL, updateGames, NULL)) {
		printf("Error Creating update Thread.\n");
		return -1;
	}


    /* start listening, allowing a queue of up to 1 pending connection */
    listen(mysocket, 1);

    while(1)
    {
		int newNetworkClient = getNewNetworkClient();
		if(newNetworkClient == -1)
		{
			printf("Too many NetworkClients connected!\n");
			close(mysocket);
			return -1;
		}

		printf("Waiting for new Client to connect.\n");
		int newSocket = accept(mysocket, (struct sockaddr*)&dest, &socksize);

		printf("New Client %d connected.\n", newNetworkClient);
		networkClients[newNetworkClient].initNetworkClient(newSocket);
		networkClients[newNetworkClient].startListener();
    }

    close(mysocket);
    return 0;
}

int startNewGame(void)
{
	//return first game thats not active
	for(int i = 0; i < maxGames; i++)
	{
		if(!activeGames[i])
		{
			activeGames[i] = true;
			return i;
		}
	}
	//return -1 when theres no game available thats not active
	return -1;
}

int getNewNetworkClient(void)
{
	for(int i = 0; i < maxNetworkClients; i++)
	{
		if(networkClients[i].clientState == CLIENT_EMPTY)
		{
			return i;
		}
	}
	return -1;
}

int getWaitingNetworkClients(NetworkClient** waitingClients)
{
	int amount = 0;
	for(int i = 0; i < maxNetworkClients; i++)
	{
		if(networkClients[i].clientState == CLIENT_INQUEUE)
		{
			waitingClients[amount] = &networkClients[i];
			amount++;
		}
	}
	return amount;
}


void* updateGames(void* info)
{
	while(1)
	{
		//Search for 2 Clients that are in the Queue, to start a new Game with them
		NetworkClient* waitingNetworkClients[maxNetworkClients];
		int waitingClientsAmount = getWaitingNetworkClients(waitingNetworkClients);
		if(waitingClientsAmount >= 2)
		{
			int newGame = startNewGame();
			if(newGame == -1)
			{
				printf("Too many Games active!\n");
			}
			else
			{
				games[newGame].newGame(2, waitingNetworkClients);
			}
		}



		//Update every active Game
		for(int i = 0; i < maxGames; i++)
		{
			if(activeGames[i])
			{
				if(games[i].update(0.02))
				{
					//Game is finished
					printf("Game %d finished.\n", i);
					activeGames[i] = false;
					games[i].freeMemory();
				}
			}
		}
		usleep(sendSleep);
	}
	return NULL;
}
 
