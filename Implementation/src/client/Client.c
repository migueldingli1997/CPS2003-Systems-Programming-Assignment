#include <stdlib.h>
#include <printf.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include "Client.h"
#include "../general/SerializeDeserialize/Serialize.h"
#include "../general/SerializeDeserialize/Deserialize.h"

int sockFD; // SOCKET DESCRIPTOR

void joinGame(const char *hostName, const int port) {

    // Find host by name
    struct hostent *server = gethostbyname(hostName);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_SUCCESS);
    }

    // Set sockFD
    sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // Set socket
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  // Set to AF_INET
    bcopy(server->h_addr, (char *) &(serv_addr.sin_addr.s_addr), (size_t) server->h_length); // Set address
    serv_addr.sin_port = htons((uint16_t) port); // Set port (convert to network byte ordering)

    // Connect to server
    if (connect(sockFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }
}

//---------------------------------------------------------------------------

Outcome recv_initialData(GameDetails **details) {

    Packet *packet;
    if (recv_packet(sockFD, &packet) == FAILED) {
        return FAILED;
    }

    // Temporary storage for the GameDetails values
    Coordinates startCoordinates;
    int xMax, yMax, numOfChanges;

    // Deserialize first part
    unsigned char *ptr = packet->data;
    ptr = deserialize_coordinates(ptr, &startCoordinates);
    ptr = deserialize_int(ptr, &xMax);
    ptr = deserialize_int(ptr, &yMax);
    ptr = deserialize_int(ptr, &numOfChanges);

    //---------------------------------------------------------------------------------------

    // Allocate accordingly and deserialize second part
    GameDetails *gd = malloc(sizeof(GameDetails) + (sizeof(MapChange) * numOfChanges));
    gd->startCoordinates = startCoordinates;
    gd->xMax = xMax;
    gd->yMax = yMax;
    gd->numOfChanges = numOfChanges;
    if (gd->numOfChanges > 0) {
        deserialize_mapChangeArray(ptr, gd->changes, gd->numOfChanges);
    }

    *details = gd;
    free(packet);
    return SUCCESSFUL;
}

Outcome recv_gameUpdate(GameUpdate **update, GameMessage *messageIndex) {

    Packet *packet;
    if (recv_packet(sockFD, &packet) == FAILED) {
        return FAILED;
    } else if (packet->header.type == PKT_GAMEUPDATE) {

        // Temporary storage for the GameUpdate values
        Coordinates snakeHead;
        int numOfChanges;

        // Deserialize first part
        unsigned char *ptr = packet->data;
        ptr = deserialize_coordinates(ptr, &snakeHead);
        ptr = deserialize_int(ptr, &numOfChanges);

        //---------------------------------------------------------------------------------------

        // Allocate accordingly and deserialize second part
        GameUpdate *gu = malloc(sizeof(GameUpdate) + (sizeof(MapChange) * numOfChanges));
        gu->snakeHead = snakeHead;
        gu->numOfChanges = numOfChanges;
        if (gu->numOfChanges > 0) {
            deserialize_mapChangeArray(ptr, gu->changes, gu->numOfChanges);
        }

        // Set gameUpdate pointer
        *update = gu;

    } else if (packet->header.type == PKT_GAMEMESSAGE) {

        // Deserialize and set message pointer
        int gm;
        deserialize_int(packet->data, &gm);
        *messageIndex = (GameMessage) gm;
    }

    free(packet);
    return SUCCESSFUL;
}

Outcome send_snakeDirection(const Direction direction) {

    // Serialize direction and send packet
    const uint packetDataSize = sizeof(Direction);
    unsigned char packetData[packetDataSize];
    serialize_int(packetData, direction); // direction essentially an integer
    return send_packet(sockFD, PKT_DIRECTION, packetDataSize, packetData);
}

//---------------------------------------------------------------------------

void exit_closeSocket(void) {

    close(sockFD);
    printf("Closing :: Called close on socket.\n");
}