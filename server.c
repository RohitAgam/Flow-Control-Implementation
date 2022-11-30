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
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<time.h>
#include<sys/types.h>     
#include<sys/socket.h>       
#include<netinet/in.h>       
#include<arpa/inet.h>
#include<unistd.h>

#define CHUNK 1024
#define FNAMESIZE 100



struct TransferInfo{
    char filename[FNAMESIZE];
    int totalPackets;
    int filesize;
    
    struct CurrentState{
        int packetNo;
        int transferCompleted;
    }currentState;
} tInfo;

struct Packet{
    int ack_flag, seq, ack;
    char buffer[CHUNK];
} packet;



int main(){

    // ---------------------- TCP code --------------------------------
    int port = 5000;
    int sockfd, clientsockfd;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(server_addr);
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error creating socket!\n");
        exit(1);
    }

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        printf("Error on setsockopt!\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Bind failed!\n");
        exit(1);
    }

    if(listen(sockfd, 1) < 0)
    {
        printf("Listen failed!\n");
        exit(1);
    }
    printf("TCP server waiting for client on port %d\n", port);

    if((clientsockfd = accept(sockfd, (struct sockaddr *) &client_addr, (socklen_t*)&addrlen))<0)
    {
        printf("Error in accept!\n");
        exit(1);
    }
    printf("Got a connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

    // ---------------------- End of TCP code --------------------------------
    
    
    FILE *file = NULL;
    int bytesrecieved = 0;

    // receive intial data about file
    if((bytesrecieved = recv(clientsockfd, &tInfo, sizeof(tInfo), 0))<0){
        printf("Error receiving info\n"); 
        exit(1);
    }
    // send ack
    packet.ack = 1;
    packet.ack_flag = 1;
    send(clientsockfd, &packet, sizeof(packet), 0);
    
    printf("Recieved tInfo:\n");
    printf("Filename: %s\n", tInfo.filename);
    printf("Transfer completed: %d\n", tInfo.currentState.transferCompleted);
    printf("Current Packet no: %d\n", tInfo.currentState.packetNo);
    printf("Total Packets: %d\n\n", tInfo.totalPackets);

    
    // transfered already
    if(tInfo.currentState.transferCompleted){
        printf("Transfer already completed!\n");
        close(clientsockfd);        
        shutdown(sockfd, SHUT_RDWR);
        return 0;
    }

    char filename[FNAMESIZE] = "Dfile_";
    strcat(filename, tInfo.filename);
    
    // partially transfered before
    if(tInfo.currentState.packetNo>0)
    {
        file=fopen(filename,"ab");
        fseek(file, tInfo.currentState.packetNo*CHUNK, SEEK_CUR);
    }
    else{ // new transfer
        printf("Creating a file named: %s\n", filename);
        file=fopen(filename,"wb");
    }

    // file transfer loop
    for(int i=tInfo.currentState.packetNo; i<tInfo.totalPackets; i++)
    {
        // receive packet
        if((bytesrecieved = recv(clientsockfd, &packet, sizeof(packet), 0))<0){
            printf("Error receiving data\n"); 
            exit(1);
        }
        printf("Receive packet %d\n", packet.seq);

        // calculate remaining length of bytes to write if this is last chunk
        int bytesToWrite = (packet.seq==tInfo.totalPackets)?tInfo.filesize%CHUNK:CHUNK; 
        
        // write to file
        fwrite(packet.buffer, 1, bytesToWrite, file);

        // send acknowledgement
        packet.ack = packet.seq+1;
        packet.ack_flag = 1;
        send(clientsockfd, &packet, sizeof(packet), 0);
        printf("Sent acknowledgement %d\n", packet.ack);

    }

    printf("done\n");
    fclose(file);
    close(clientsockfd);        
    shutdown(sockfd, SHUT_RDWR);
    
    return 0;
}
server14.c
Displaying server14.c.