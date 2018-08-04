#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "General.h"
#include "SerializeDeserialize/Deserialize.h"
#include "SerializeDeserialize/Serialize.h"

const char *GameMessages[TOTAL_MESSAGES] = {
    "Welcome to the game!",
    "You ate a fruit and your length was increased!",
    "You collided! You are now in spectator mode.",
    "You won the game! The game is being restarted...",
    "Another snake won! The game is being restarted...",
    "Starting game...3!",
    "Starting game...2!",
    "Starting game...1!",
    "GO!!",
    "Server closed down. Game will now close."
};

void terminationHandler(int n) {

    if (n == SIGTSTP || n == SIGINT) {
        exit(EXIT_SUCCESS);
    }
}

static Outcome checkOperationResult(const ssize_t result) {

    if (result <= 0) {
        if (errno == EPIPE || errno == ECONNRESET) {
            //printf("Socket disconnected.\n");
            return FAILED;
        } else if (result == 0) {
            //printf("Read/Wrote 0 bytes. Assuming that socket disconnected.\n");
            return FAILED;
        } else {
            perror("Unhandled error when accessing socket");
            exit(EXIT_FAILURE);
        }
    }
    return SUCCESSFUL;
}

static Outcome readLoop(const int sockFD, unsigned char *buffer, const size_t bytesToRead) {

    ssize_t bytesRead = 0;
    do {
        const ssize_t readResult = read(sockFD, buffer + bytesRead, (bytesToRead - bytesRead));
        if (checkOperationResult(readResult) == FAILED) {
            return FAILED;
        } else {
            bytesRead += readResult;
        }
    } while (bytesRead < bytesToRead);
    return SUCCESSFUL;
}

static Outcome writeLoop(const int sockFD, unsigned char *buffer, const size_t bytesToWrite) {

    ssize_t bytesWritten = 0;
    do {
        const ssize_t writeResult = write(sockFD, buffer + bytesWritten, bytesToWrite - bytesWritten);
        if (checkOperationResult(writeResult) == FAILED) {
            return FAILED;
        } else {
            bytesWritten += writeResult;
        }
    } while (bytesWritten < bytesToWrite);
    return SUCCESSFUL;
}

Outcome recv_packet(const int sockFD, Packet **packet) {

    // Read packet header
    unsigned char headerBuffer[sizeof(PacketHeader)];
    if (readLoop(sockFD, headerBuffer, sizeof(PacketHeader)) == FAILED) {
        return FAILED;
    }

    // Deserialize packet header details
    const PacketType packetType;
    const uint packetSize;
    unsigned char *ptr = headerBuffer;
    ptr = deserialize_int(ptr, (int *) &packetType);
    ptr = deserialize_int(ptr, (int *) &packetSize);

    // Set packet header and read packet data
    Packet *pkt = malloc(sizeof(PacketHeader) + (packetSize * sizeof(char)));
    pkt->header.type = packetType;
    pkt->header.size = packetSize;
    readLoop(sockFD, pkt->data, packetSize);

    *packet = pkt;
    return SUCCESSFUL;
}

Outcome send_packet(const int sockFD, PacketType packetType, uint packetSize, unsigned char *packetData) {

    // Serialize packet header details
    unsigned char headerBuffer[sizeof(PacketHeader)];
    unsigned char *ptr = headerBuffer;
    ptr = serialize_int(ptr, packetType);
    ptr = serialize_int(ptr, packetSize);

    // Write packet header and packet data
    if (writeLoop(sockFD, headerBuffer, sizeof(PacketHeader)) == FAILED) {
        return FAILED;
    }
    if (writeLoop(sockFD, packetData, packetSize) == FAILED) {
        return FAILED;
    }
    return SUCCESSFUL;
}

bool coordEqual(const Coordinates *c1, const Coordinates *c2) {
    return (c1->x == c2->x) && (c1->y == c2->y);
}