2022-2023 Networking
M.Tech (Lab)
Assignment details
Assignment 4: Flow Control Implementation
Sujoy Saha
•
Oct 29 (Edited Nov 3)
100 points
Due Nov 5, 11:59 PM
Implement naïve flow control mechanism using stop & wait protocol. Transfer files (Text, Image, Audio, Video) using TCP and UDP protocol. If during the connection suddenly connection is terminated then you have start ones again, it simply resume the process not start from beginning.

Write a socket program in C/Java for Multimodal File Transmission using TCP and UDP with Full-Duplex Stop and Wait protocol. The program/protocol should support the following properties/mechanism
1.   The protocol will send any type of files
2.   Each packet should consist of the file name, sequence number/Acknowledgement number
3.  A log file should be generated with some information like, 

 List of uncommon files in server and client which are to be transferred, Start time, If the connection is broken then the % of the file already uploaded, How many times connections were established during the complete transmission, End time (when the file is fully transmitted), How many packets are lost, How many time-outs are occurred, etc.

Assignment 3.docx
Word

Socket Programming.mp4
Video
Class comments
Your work
Turned in

server14.c
C

client14.c
C
Private comments
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define CHUNK 1024
#define FNAMESIZE 100



struct TransferInfo
{
    char filename[FNAMESIZE];
    int totalPackets;
    int filesize;

    struct CurrentState
    {
        int packetNo;
        int transferCompleted;
    } currentState;
} tInfo;

struct Packet{
    int ack_flag, seq, ack;
    char buffer[CHUNK];
} packet;

int filesize(FILE *fp)
{
    int loc = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, loc, SEEK_SET);
    return sz;
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Please enter a filename\n");
        exit(1);
    }
    char filename[FNAMESIZE];
    strcpy(filename, argv[1]);

    // ------------------------------- TCP code -----------------------------

    char server_ip[18] = "127.0.0.1";
    int port = 5000;
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(server_addr);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error creating socket!\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        printf("Invalid address\n");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("Error creating connection!\n");
        exit(1);
    }

    // ------------------------------- End of TCP code -----------------------------

    char logFileName[FNAMESIZE];
    strcpy(logFileName, filename);
    strcat(logFileName, ".log");

    FILE *logfile, *file;

    // load data from log file if it exists
    if (access(logFileName, F_OK) == 0)
    {
        printf("Log file already exists!\n");
        logfile = fopen(logFileName, "rb");
        fread(&tInfo, sizeof(tInfo), 1, logfile);
        fclose(logfile);

        // transfer completed already before
        if (tInfo.currentState.transferCompleted)
        {
            printf("Transfer already completed!\n");
            send(sockfd, &tInfo, sizeof(tInfo), 0); // send metadata
            close(sockfd);
            return 0;
        }

        // partially transfered before
        else
        {
            printf("Partially transfered before %d/%d!\n", tInfo.currentState.packetNo, tInfo.totalPackets);
            file = fopen(filename, "rb");
            fseek(file, tInfo.currentState.packetNo * CHUNK, SEEK_SET);
        }
    }
    else
    { // first time transfer

        file = fopen(filename, "rb"); // open file

        // load info about file into transfer info struct
        strcpy(tInfo.filename, filename);
        tInfo.filesize = filesize(file);
        tInfo.totalPackets = (tInfo.filesize+CHUNK-1)/CHUNK; // ceil
        tInfo.currentState.packetNo = 0;
        tInfo.currentState.transferCompleted = 0;
    }

    int bytesrecieved = 0, bytesread = 0;

    // send file information
    send(sockfd, &tInfo, sizeof(tInfo), 0);

    // recv ack
    if (!(recv(sockfd, &packet, sizeof(packet), 0) > 0 && packet.ack_flag && packet.ack == 1))
    {
        printf("Error receiving information about file\n");
        exit(1);
    }

    logfile = fopen(logFileName, "wb");

    // file transfer loop
    for (int seq = tInfo.currentState.packetNo + 1; seq <= tInfo.totalPackets; seq++)
    {
        if ((bytesread = fread(packet.buffer, 1, CHUNK, file)) <= 0)
        {
            printf("Issue in reading file\n");
            exit(1);
        }
        packet.ack_flag = 0;
        packet.seq = seq;

        send(sockfd, &packet, sizeof(packet), 0);
        printf("Sent packet %d/%d\n", seq, tInfo.totalPackets);

        // recv ack
        if ((bytesrecieved = recv(sockfd, &packet, sizeof(packet), 0)) < 0)
        {
            printf("Error receiving data\n");
            exit(1);
        }

        if (packet.ack_flag)
        {
            printf("Received acknowledgement %d\n", packet.ack);

            // write to log file
            tInfo.currentState.packetNo = seq;
            if (seq == tInfo.totalPackets) {
                printf("Transfer Completed\n");
                tInfo.currentState.transferCompleted = 1;
            }
            fseek(logfile, 0, SEEK_SET);
            fwrite(&tInfo, sizeof(struct TransferInfo), 1, logfile);
        }
    }

    fclose(file);
    fclose(logfile);
    printf("done\n");

    close(sockfd);

    return 0;
}
client14.c
Displaying client14.c.