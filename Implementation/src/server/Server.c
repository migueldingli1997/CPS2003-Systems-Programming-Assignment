#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "Server.h"
#include "Game/Game.h"
#include "../general/SerializeDeserialize/Deserialize.h"
#include "../general/SerializeDeserialize/Serialize.h"

int serverSFD; // SOCKET DESCRIPTOR

void startListeningForClients(const int port) {

    // Create server socket (AF_INET, SOCK_STREAM);
    struct sockaddr_in serv_addr;
    serverSFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSFD < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // Initialize socket structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address
    serv_addr.sin_port = htons((uint16_t) port); // Set server port

    // Bind host address
    if (bind(serverSFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }

    // Listen for clients
    listen(serverSFD, REQ_QUEUE_SIZE);
    printf("Server started on port %d and is now listening for clients...\n", (uint16_t) port);
}

int acceptConnection(struct sockaddr_in *cli_addr, const size_t clilen) {

    while (true) {
        const int newClientSFD = accept(serverSFD, (struct sockaddr *) cli_addr, (socklen_t *) &clilen);
        if (newClientSFD < 0) {
            fprintf(stderr, "ERROR on accept\n"); // Server does not crash if accept fails
        } else {
            return newClientSFD;
        }
    }
}

//---------------------------------------------------------------------------

Outcome send_gameDetails(const int clientSFD, const GameDetails *gd, const List *snakesList, const Coordinates fruit) {

    // Serialize game details and send packet
    const uint packetDataSize = SIZEOF_GameDetailsFirstPart + (SIZEOF_MapChange * gd->numOfChanges);
    unsigned char packetData[packetDataSize];
    serialize_gameDetails(packetData, gd, snakesList, fruit);
    return send_packet(clientSFD, PKT_GAMEDETAILS, packetDataSize, packetData);
}

Outcome send_gameMessage(const int clientSFD, const GameMessage msg) {

    // Send packet (no serialization required)
    const uint packetDataSize = sizeof(int);
    unsigned char packetData[packetDataSize];
    serialize_int(packetData, (int) msg);
    return send_packet(clientSFD, PKT_GAMEMESSAGE, packetDataSize, packetData);
}

Outcome send_gameMessageToAll(const List *snakesList, const GameMessage msg) {

    Node *node = snakesList->first;
    while (node != NULL) {
        send_gameMessage(((Snake *) node->value)->clientSFD, msg);
        node = node->next;
    }
    return SUCCESSFUL; // send_gameMessage failures not handled
}

Outcome send_gameUpdates(List *snakesList, const List *primaryChanges, List *secondaryChanges) {

    // Serialize game details
    const uint packetDataSize = SIZEOF_GameUpdateFirstPart + (SIZEOF_MapChange * primaryChanges->nodeCount);
    unsigned char packetData[packetDataSize];
    serialize_gameUpdate(packetData, primaryChanges);

    // Send personalized packets to snakes
    Node *node = snakesList->first;
    while (node != NULL) {
        Snake *snake = (Snake *) node->value;

        // Personalized based on snake head coordinates
        serialize_coordinates(packetData, snake->head->coord);
        if (send_packet(snake->clientSFD, PKT_GAMEUPDATE, packetDataSize, packetData) == FAILED) {

            pthread_mutex_lock(&snakesListAndFruitLock); // Lock snake list and fruit

            // Clear snake from map and from snakes list
            clearPartsFromMap(snake->head, secondaryChanges);
            Node *toRemove = node;
            node = node->next; // this statement's placement is important
            clearNodeFromList(snakesList, (freeNodeValueFn) &freeSnake, toRemove);
            printf("Client left the game (Reason: sending of update failed). "
                       "New client count: %d.\n", snakesList->nodeCount);

            pthread_mutex_unlock(&snakesListAndFruitLock); // Unlock snake list and fruit

        } else {
            node = node->next;
        }
    }
    return SUCCESSFUL;
}

Outcome recv_direction(const int clientSFD, Direction *direction) {

    Packet *packet;
    if (recv_packet(clientSFD, &packet) == FAILED) {
        return FAILED;
    }
    deserialize_int(packet->data, (int *) direction); // direction essentially an integer
    free(packet);
    return SUCCESSFUL;
}

void exit_closeServerSocket(void) {

    close(serverSFD);
    printf("Closing :: Called close on server socket.\n");
}