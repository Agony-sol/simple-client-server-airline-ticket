#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SIZE 1024

int main(int argc, char *argv[]){
    int sock, i, num;
    char msg[SIZE];
    struct sockaddr_un addr;
    socklen_t addrlen;

    //CREATE AND CONNECT TO PARENT SOCKET
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);
    addrlen = sizeof(addr.sun_family) + strlen(addr.sun_path) + 1;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr*)&addr, addrlen);

    //SEND CUSTOMER INFO
    printf("Please Type Name\n");
    scanf("%s", msg);
    send(sock, msg, strlen(msg)+1, 0);
    strcpy(msg, "");

    printf("Please Type Surname\n");
    scanf("%s", msg);
    send(sock, msg, strlen(msg)+1, 0);
    strcpy(msg, "");

    printf("Please Type Passport Country\n");
    scanf("%s", msg);
    send(sock, msg, strlen(msg)+1, 0);
    strcpy(msg, "");

    printf("Please Type Passport Number\n");
    scanf("%s", msg);
    send(sock, msg, strlen(msg)+1, 0);
    strcpy(msg, "");

    //SEND DESIRABLE FLIGHT INFO
    printf("Please Type Destination\n");
    scanf("%s", msg);
    send(sock, msg, strlen(msg)+1, 0);
    strcpy(msg, "");

    printf("Please type City of Origin\n");
    scanf("%s", msg);
    send(sock, msg, strlen(msg)+1, 0);
    strcpy(msg, "");


    //RECEIVE FLIGHT SCHEDULE FROM SERVER
    recv(sock, msg, SIZE, 0);
    if(strcmp(msg, "No flights found for this route.\n")==0){ //check if flight is available else exit
        printf("%s", msg);
        exit(0);
    }
    printf("List of Available Flights:\n");
    printf("\n%s\n", msg);

    //SEND PREFERED FLIGHT TO SERVER
    printf("\nPlease Type The Desired Flight Number...\n");
    strcpy(msg, "");
    scanf("%d", &num);
    if(num > 15 || num < 1){
        printf("\nWrong Flight Number. Please Try Again Later...\n");
        exit(0);
    }
    send(sock, &num, sizeof(num), 0);


    //RECEIVE AVAILABLE SEATS1 FROM SERVER
    printf("\nList of Available Seats:\n");
    int flight2=-1;
    recv(sock, &flight2, sizeof(flight2), 0); //Check if there is flight 2.
    recv(sock, msg, SIZE, 0);
    printf("\n%s\n", msg);
    strcpy(msg, "");

    //SEND SEAT1 PREFERENCE
    printf("\nPlease Choose a Seat By typing the Number...\n");
    scanf("%d", &num);
    if(num > 10 || num < 1){
        printf("\nWrong Seat Number. Please Try Again Later...\n");
        exit(0);
    }
    send(sock, &num, sizeof(num), 0);

    if(flight2 != -1){
        //RECEIVE AVAILABLE SEATS2 FROM SERVER
        printf("\nList of Available Seats For Second Flight:\n");
        recv(sock, msg, SIZE, 0);
        printf("\n%s\n", msg);
        strcpy(msg, "");

        //SEND SEAT2 PREFERENCE
        printf("\nPlease Choose a Seat By typing the Number...\n");
        scanf("%d", &num);
        if(num > 10 || num < 1){
            printf("\nWrong Seat Number. Please Try Again Later...\n");
            exit(0);
        }
        send(sock, &num, sizeof(num), 0);
    }



    recv(sock, msg, SIZE, 0);
    printf("\n%s\n", msg);

    close(sock);
    return 0;

}