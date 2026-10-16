//ΓΙα να μην ξεχασω: gcc server.c -o server -pthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 1024

void *buy_ticket(void *arg);

//Flight Schedule
typedef struct{
    char origin[30];
    char departure_date[30];
    char departure_time[30];
    char destination[30];
    char arrival_date[30];
    char arrival_time[30];
    int seats[10];
    pthread_mutex_t lock;
}Flight;


Flight availableFlights[] = {
    {"Athens","25/4/2026","13:00","Budapest","25/4/2026","15:00"},
    {"Athens","29/4/2026","08:00","London","29/4/2026","12:00"},
    {"London","29/4/2026","13:00","Budapest","29/4/2026","15:00"},
    {"Athens", "30/4/2026", "09:00", "Rome", "30/4/2026", "10:30"},
    {"Rome", "30/4/2026", "12:00", "Paris", "30/4/2026", "14:00"},
    {"Paris", "30/4/2026", "16:00", "Madrid", "30/4/2026", "18:00"}, 
    {"Athens", "01/5/2026", "11:00", "Paris", "01/5/2026", "13:30"},
    {"London", "01/5/2026", "14:00", "Berlin", "01/5/2026", "16:00"},
    {"Berlin", "01/5/2026", "17:30", "Warsaw", "01/5/2026", "19:00"}, 
    {"Athens", "02/5/2026", "08:30", "Madrid", "02/5/2026", "11:30"},
    {"Madrid", "02/5/2026", "14:00", "Lisbon", "02/5/2026", "14:30"},
    {"Rome", "03/5/2026", "18:00", "Athens", "03/5/2026", "21:00"}, 
    {"Athens", "04/5/2026", "07:00", "Vienna", "04/5/2026", "08:30"},
    {"Vienna", "04/5/2026", "11:00", "Prague", "04/5/2026", "12:00"},
    {"Budapest", "04/5/2026", "09:00", "Vienna", "04/5/2026", "10:00"}
};



int main(int argc, char* argv[]){

    //Create/Bind Socket
    int psock, sock, i, j, num, opt=1, ret;
    struct sockaddr_un addr;
    socklen_t addrlen;
    
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);
    addrlen = sizeof(addr.sun_family) + strlen(addr.sun_path) + 1;
    psock = socket(AF_UNIX, SOCK_STREAM, 0);
    bind(psock,(struct sockaddr*)&addr,addrlen);
    listen(psock,10);

    int length = sizeof(availableFlights) / sizeof(availableFlights[0]);

    //Make all Seats Available on Startup
    for(j=0; j<length; j++){
        for(i=0; i<10; i++){
            availableFlights[j].seats[i] = 0;
            pthread_mutex_init(&availableFlights[j].lock, NULL);
        }
    }
    
    FILE *file = fopen("flights.txt", "w");
    if (file != NULL) {
        fclose(file);
    }

    //Create Thread
    pthread_t thread;

    while(1){

        //Establish Connection With Client
        printf("\nWaiting for new connection...\n");
        sock = accept(psock, NULL, NULL);
        ret = pthread_create(&thread, NULL, buy_ticket, &sock);    
        pthread_detach(thread);
    }

    close(psock);
    unlink(argv[1]);
    return 0;
}




void *buy_ticket(void *arg){

    int length = sizeof(availableFlights) / sizeof(availableFlights[0]);
    int sock = *(int *)arg; //we take the sock from the argumen from pthread_create
    int i, j, num, opt=1, ret;
    char msg[SIZE], orgn[SIZE], dest[SIZE], name[SIZE], surname[SIZE], country[SIZE], passport[SIZE];
    int flights[length][2];

    
    printf("\nConnection Established...\n");
    char temp_msg[256];
    strcpy(msg, "");

    //GET CUSTOMER INFO
    recv(sock, name, SIZE, 0);
    printf("\nName: %s\n", name);
    recv(sock, surname, SIZE, 0);
    printf("\nSurname: %s\n", surname);
    recv(sock, country, SIZE, 0);
    recv(sock, passport, SIZE, 0);


    //GET FLIGHT INFO
    recv(sock, dest, SIZE, 0);
    printf("\nChosen Destination: %s\n", dest);
    recv(sock, orgn, SIZE, 0);
    printf("\nChosen origin: %s\n", orgn);

    //Send Flight Schedule to Client.
    strcpy(msg, "");
    for(int i=0; i<length; i++){
        if(strcmp(orgn, availableFlights[i].origin) == 0 && strcmp(dest, availableFlights[i].destination) == 0){ //First If checks direct flights
            sprintf(temp_msg, "%d. [DIRECT] From: %s To %s (%s %s)\n",
                    opt,
                    availableFlights[i].origin,
                    availableFlights[i].destination,
                    availableFlights[i].departure_date,
                    availableFlights[i].departure_time);
            
            strcat(msg, temp_msg);
            flights[opt-1][0] = i;
            flights[opt-1][1] = -1; //if flight is direct no second flight (-1)
            opt++;

        }else if(strcmp(orgn, availableFlights[i].origin) == 0){ //This if scans all destinations from specific origin
            for (int j=0; j<length; j++){
                if(strcmp(availableFlights[i].destination, availableFlights[j].origin) == 0 && strcmp(dest, availableFlights[j].destination) == 0){ //This if comparesa destination of first flight to origin of second flight and shows those with same final dest.
                sprintf(temp_msg, "%d. [LAYOVER] From: %s To %s To %s\n   Leg 1: %s %s\n   Leg 2: %s %s\n",
                            opt,
                            availableFlights[i].origin, 
                            availableFlights[i].destination, 
                            availableFlights[j].destination,
                            availableFlights[i].departure_date, availableFlights[i].departure_time,
                            availableFlights[j].departure_date, availableFlights[j].departure_time);

                            strcat(msg, temp_msg);
                            flights[opt-1][0] = i;
                            flights[opt-1][1] = j;
                            opt++;
                }
            }
        }
    }

    if(strlen(msg) == 0){
        strcpy(msg, "No flights found for this route.\n");
    }

    send(sock, msg, strlen(msg)+1, 0);
    if(strcmp(msg, "No flights found for this route.\n") == 0){
        close(sock);
        return NULL;
    }

    //Receive From Client Flight Selection
    recv(sock, &num, sizeof(num), 0);
    int prefered_flight = num-1;
    printf("\nChosen Flight: %d\n", num);

    int flight1 = flights[prefered_flight][0];
    int flight2 = flights[prefered_flight][1];

    //Send Client Available Seats For Specified Flight
    strcpy(msg, "");
    for(i=0; i<10; i++){
        if(availableFlights[flight1].seats[i] != 1){
            sprintf(temp_msg, "Seat: %d", i+1);
            strcat(msg, temp_msg);
            strcat(msg, "\n");
        }
    }
    send(sock, &flight2, sizeof(flight2), 0);
    send(sock, msg, strlen(msg)+1, 0);


    //Receive Seat Preference
    recv(sock, &num, sizeof(num), 0);
    int prefered_seat1 = num-1;
    printf("\nChosen Seat: %d\n", num);

    strcpy(temp_msg, "");
    strcpy(msg, "");

    int prefered_seat2;
    if(flight2 != -1){
        for(i=0; i<10; i++){
            if(availableFlights[flight2].seats[i] != 1){
                sprintf(temp_msg, "Seat: %d", i+1);
                strcat(msg, temp_msg);
                strcat(msg, "\n");
            }
        }
        send(sock, msg, strlen(msg)+1, 0);

        recv(sock, &num, sizeof(num), 0);
        prefered_seat2 = num-1;
        printf("\nChosen Seat: %d\n", num);
    }

    //We lock only desired flights
    pthread_mutex_lock(&availableFlights[flight1].lock);
    if(flight2 != -1){
        pthread_mutex_lock(&availableFlights[flight2].lock);
    }  


    //Check Seat Availability
    if(availableFlights[flight1].seats[prefered_seat1] != 1){
        if(flight2 == -1){
            availableFlights[flight1].seats[prefered_seat1] = 1;
            strcpy(msg, "");
            strcpy(msg, "Thanks For Choosing Us.");
            send(sock, msg, strlen(msg)+1, 0);
            FILE *file = fopen("flights.txt", "a");
            if (file != NULL) {
                fprintf(file, "DIRECT! Από %s Προς %s: %s %s Passport: %s %s\n", orgn, dest, name, surname, country, passport);
                fclose(file);
            }
        }
    }else{
        strcpy(msg, "");
        strcpy(msg, "Seat already taken.");
        send(sock, msg, strlen(msg)+1, 0);
    }

    if(flight2 != -1){
        if(availableFlights[flight2].seats[prefered_seat2] != 1 && availableFlights[flight1].seats[prefered_seat1] != 1){
            availableFlights[flight1].seats[prefered_seat1] = 1;
            availableFlights[flight2].seats[prefered_seat2] = 1;
            strcpy(msg, "");
            strcpy(msg, "Thanks For Choosing Us.");
            send(sock, msg, strlen(msg)+1, 0);
            FILE *file = fopen("flights.txt", "a");
            if (file != NULL) {
                fprintf(file, "LAYOVER! Από %s Προς %s: %s %s Passport: %s %s\n", orgn, dest, name, surname, country, passport);
                fclose(file);
            }
        }else{
            strcpy(msg, "");
            strcpy(msg, "Second Seat already taken.");
            send(sock, msg, strlen(msg)+1, 0);
        }
    }

    //And now we unlock them
    pthread_mutex_unlock(&availableFlights[flight1].lock);
    if(flight2 != -1){
        pthread_mutex_unlock(&availableFlights[flight2].lock);
    } 

    close(sock);
    return NULL;
}