/*
 * NetworkClient.cpp
 *
 *  Created on: 05.01.2018
 *      Author: david
 */

#include "NetworkClient.h"


void NetworkClient::sendBuffer()
{
	send(clientSocket, buffer, bufferPosition, 0);
	bufferPosition = 0;
}

void NetworkClient::addInformationToBuffer(unsigned int type, void* informationPtr, unsigned int size)
{
	memcpy(buffer + bufferPosition, &type, sizeof(type));
	bufferPosition += sizeof(type);
	memcpy(buffer + bufferPosition, informationPtr, size);
	bufferPosition += size;
}

void NetworkClient::initNetworkClient(int clientSocket) {
	this->clientSocket = clientSocket;
	listenerThread = 0;
	clientState = CLIENT_CONNECTED;
}


int NetworkClient::startListener()
{
	if(pthread_create(&listenerThread, NULL, listenToClient, this)) {
		printf("Error Creating Thread.\n");
		return -1;
	}
	return 0;
}

int NetworkClient::linkWithGame(struct PanzerInformation* myPanzerInformation, struct ProjectileInformation* myNewProjectile, int* myPanzerHealth, int clientNumber)
{
	this->myPanzerInformation = myPanzerInformation;
	this->myNewProjectile = myNewProjectile;
	this->myPanzerHealth = myPanzerHealth;
	this->clientNumber = clientNumber;
	clientState = CLIENT_INGAME;
	return 0;
}

int NetworkClient::seperateFromGame()
{
	this->myPanzerInformation = NULL;
	this->myNewProjectile = NULL;
	this->myPanzerHealth = NULL;
	clientState = CLIENT_VERIFIED;
	return 0;
}

void NetworkClient::sendGameInformation(int playerAmount)
{
	struct GameInformation gameInformation;
	gameInformation.myNumber = clientNumber;
	gameInformation.playerAmount = playerAmount;

	printf("Sending Game Information to %d: %d Player.\n", gameInformation.myNumber, gameInformation.playerAmount);

	addInformationToBuffer(1, &gameInformation, sizeof(gameInformation));
	sendBuffer();
}

void NetworkClient::sendRemoveProjectile(struct ProjectileInformation* projectile)
{
    addInformationToBuffer(200 + projectile->parent, projectile, sizeof(struct ProjectileInformation));
}

void NetworkClient::sendOtherPanzer(struct PanzerInformation* otherPanzer, int* panzerHealth, unsigned int playerAmount)
{
	addInformationToBuffer(50, &panzerHealth[clientNumber], sizeof(panzerHealth[0]));
	for(unsigned int i = 0; i < playerAmount; i++)
	{
		//Index soll über eigenen Panzer springen
		if(i != clientNumber)
		{
			otherPanzer[i].health = panzerHealth[i];
			addInformationToBuffer(100 + i, &otherPanzer[i], sizeof(PanzerInformation));
		}
	}
}

void NetworkClient::sendGameFinishInformation(struct GameFinishInformation* information)
{
	addInformationToBuffer(70, &information, sizeof(struct GameFinishInformation));
	sendBuffer();
}

void NetworkClient::sendNewProjectile(struct ProjectileInformation* projectileInformation)
{
	addInformationToBuffer(1000 + projectileInformation->parent, projectileInformation, sizeof(struct ProjectileInformation));
}

void* NetworkClient::listenToClient(void* parent)
{
	NetworkClient* myClient = (NetworkClient*) parent;

	printf("New Thread for Client %d created!\n", myClient->clientSocket);

	char buffer[bufferSize] = {0};

	while(1)
	{
		unsigned int length;
		if((length = read(myClient->clientSocket, buffer, bufferSize)) <= 0)
		{
			printf("Problem receiving message: Client %d disconnected.!\n", myClient->clientSocket);
			myClient->clientState = CLIENT_DISCONNECTED;
			break;
		}

		unsigned int messageType;

		unsigned int locInBuf = 0;
		while(length >= (locInBuf + sizeof(messageType)))
		{
			messageType = *((unsigned int*) &buffer[locInBuf]);

			locInBuf += sizeof(messageType);

			switch(messageType)
			{
				case 0:
				{
					//Player Name received
					if(length < sizeof(playerName)) //Message too short
					{
						printf("Received Player Name too short!\n");
					}
					memcpy(myClient->playerName, &buffer[locInBuf], sizeof(myClient->playerName));
					if(playerName_verify(myClient->playerName) == 0)
					{
						printf("Player Name %s verfied.\n", myClient->playerName);

						//Player Name is valid
						myClient->clientState = CLIENT_VERIFIED;

						//Send Reply
						char replyText[] = "Hello Client!";

						myClient->addInformationToBuffer(0, replyText, 14);

						myClient->sendBuffer();
					}
					else
					{
						myClient->clientState = CLIENT_CONNECTED;
					}

					locInBuf += playerName_Length;
					break;
				}

				case 5:
				{
					//Player wants to enter queue
					if(myClient->clientState == CLIENT_VERIFIED)
					{
                        printf("Player entered Queue.\n");
						myClient->clientState = CLIENT_INQUEUE;
					}
					break;
				}

				case 99:
				{
                    //New Panzer Information received
                    if(myClient->clientState == CLIENT_INGAME)
                    {
                        if(length < locInBuf + sizeof(struct PanzerInformation)) //Message too short
                        {
                            printf("Received Panzer too short.\n");
                            locInBuf = length;
                            break;
                        }
						memcpy(myClient->myPanzerInformation, &buffer[locInBuf], sizeof(struct PanzerInformation));
                    }
					locInBuf += sizeof(struct PanzerInformation);
					break;
				}
				case 98:
				{
                    //New Projectile received
                    if(myClient->clientState == CLIENT_INGAME)
                    {
                        if(length < locInBuf + sizeof(struct ProjectileInformation))
                        {
                            printf("Received Projectile too short.\n");
                            locInBuf = length;
                            break;
                        }
                        memcpy(myClient->myNewProjectile, &buffer[locInBuf], sizeof(ProjectileInformation));
                    }
					locInBuf += sizeof(struct ProjectileInformation);
					break;
				}
				default:
				{
					printf("Unknown Message Type %d received.\n", messageType);
					locInBuf = length;
					break;
				}
			}
		}
	}

	if(myClient->myPanzerHealth != NULL)
	{
		*(myClient->myPanzerHealth) = -1;
	}

	closeSocketDef(myClient->clientSocket);
	myClient->seperateFromGame();
	myClient->clientState = CLIENT_EMPTY;
	return NULL;
}

NetworkClient::~NetworkClient()
{

}
