#include "Server.h"
#include "Helper.h"

s_node *head;
void printList(s_node *head) {
    s_node *current_node = head;
    while(current_node != NULL) {
        printf("(%d|%d|%d|%s) --> ", current_node->s_i, current_node->s_j, current_node->s_code, current_node->usr);
        current_node = current_node->next;
    }
    puts("(NULL)\n");
}

void *thread_func(void *param) {

    int c_socket = (long int)param;
    char *username, *usernameV;
    char *password, *passwordV;
    char buffer[1024];
    char tempBuf[1024];
    char bufferS[1024];
    char bufferB[1024];
    int seatCheck = 0;
    char reservedSeats[1024];
    int curr_n_seats;
    s_node *curr;
    int x, y, fd, countV = 0;
    char *p1, *p2;
    ssize_t ret;

    struct timeval timeout;
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    if(setsockopt(c_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("[-]Setsockopt failed.\n");
        close(c_socket);
        pthread_exit(NULL);
    }

    printList(head);

    login_start:
    ret = safeRead(c_socket, buffer, MAX_DIM - 1);
    handle_error_rs(ret, c_socket);

    buffer[strcspn(buffer, "\n")] = 0;

    operation = atoi(buffer);

    printList(head);

    switch(operation) {
//
//CASE 1: LOGIN
//
	 case 1:
         puts("CASE 1 LOGIN");
            ret = safeRead(c_socket, buffer, MAX_DIM - 1);
            handle_error_rs(ret, c_socket);

            buffer[strcspn(buffer, "\n")] = 0;

            strcpy(bufferS, buffer);

            username = strtok(bufferS, ";");

            password = strtok(NULL, ";");

            fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);	//Opening file to read datas
            fileL = fdopen(fd, "w+");

            countV = 0;

            while(fgets(tempBuf, sizeof(tempBuf), fileL)) {             //Checking all fs-database entries for corresponding username and password

                tempBuf[strcspn(tempBuf, "\n")] = 0;

                usernameV = strtok(tempBuf, ";");

                passwordV = strtok(NULL, ";");

                if (!strcmp(username, usernameV)) {                     //Checking the username with the corresponding entry
                    if (!strcmp(password, passwordV)){                  //Checking the password with the corresponding entry
                        countV = 1;
                        break;
                    }
                }
            }

            if(countV) {                                                //Account found, sending ack to client
                sprintf(buffer, "%s\n", "1");
                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);


            } else if (!countV) {					//Acccount not found, sending ack to client
                sprintf(buffer, "%s\n", "0");
                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);
                goto login_start;					//Since account hasn't been found, user can retry to log-in or sign-up
            }
            break;
//
//CASE 2:SIGN-UP
//
        case 2:
            puts("CASE 2 LOGIN");
            ret = safeRead(c_socket, buffer, MAX_DIM - 1);
            handle_error_rs(ret, c_socket);

            buffer[strcspn(buffer, "\n")] = 0;

            fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
            fileL = fdopen(fd, "w+");

            if(fileL == NULL) {
                puts("[-]Fopen failed\n");
                pthread_exit(NULL);
            }

            strcpy(bufferS, buffer);

            username = strtok(bufferS, ";");

            countV = 0;

            if (pthread_mutex_lock(login)) {
                perror("[-]Mutex lock error");
                pthread_exit(NULL);
            }

            while(fgets(tempBuf, sizeof(tempBuf), fileL) && !countV) {	//Checking all fs-db entries to see if the username already exists

                puts("inside while");

                tempBuf[strcspn(tempBuf, "\n")] = 0;

                usernameV = strtok(tempBuf, ";");

                if (!strcmp(username, usernameV)) {
                    countV++;
                } else {
                    countV = 0;
                }
            }

            fclose(fileL);

            if(!countV) {						//Username doesn't exist in fs-db so the user can register with said username	
                fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
                fileL = fdopen(fd, "w+");
                fprintf(fileL, "%s\n",  buffer);			//Insert username and password into fs-db
                fflush(fileL);
                fclose(fileL);
                if (pthread_mutex_unlock(login)) {
                    perror("[-]Mutex unlock error");
                    pthread_exit(NULL);
                }
                sprintf(buffer, "%s\n", "1");				//Sending ack to client to confirm the registration
                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);
            } else {
                sprintf(buffer, "%s\n", "0");				//Sending ack to client saying the username is already taken
                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);
                if (pthread_mutex_unlock(login)) {
                    perror("[-]Mutex unlock error");
                    pthread_exit(NULL);
                }
                goto login_start;					//Since username was taken, user can retry to sign-up or login
            }
            break;
//
//CASE 3: EXIT
//
        case 3:
            pthread_exit(NULL);
    }

    sprintf(buffer, "%d;%d\n", rows, r_seats);

    ret = safeWrite(c_socket, buffer, strlen(buffer));
    handle_error_ws(ret, c_socket);

    while(1){
        puts("[!]Waiting for an input\n");
        printList(head);
        ret = safeRead(c_socket, buffer, MAX_DIM - 1);
        handle_error_rs(ret, c_socket);

        buffer[strcspn(buffer, "\n")] = 0;

        operation = atoi(buffer);

        switch(operation) {
//
//CASE 1: PRINT THE MAP OF THE SEATS
//
            case 1:
                puts("CASE 1 T");
                sprintf(buffer, "%d\n", n_counter);			//Sending the number of the current reserved seats to client
                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);

                printf("[+]Sending data to client.\n");			//Sending the coordinates of the reserved seats to the client
                curr = head;
                while(curr->s_j >= 0 && curr->s_i >= 0 && curr) {
                    sprintf(buffer, "%d;%d\n", curr->s_i, curr->s_j);
                    ret = safeWrite(c_socket, buffer, strlen(buffer));
                    handle_error_ws(ret, c_socket);
                    curr = curr->next;
                }
                puts("[+]Data to client successfully sent.\n");
                break;
//
//CASE 2: RESERVE A SEAT
//
            case 2:
                puts("CASE 2 T");
            reserv_routine:
                ret = safeRead(c_socket, buffer, MAX_DIM - 1);
                handle_error_rs(ret, c_socket);

                buffer[strcspn(buffer, "\n")] = 0;

                tokind = atoi(buffer);


                ret = safeRead(c_socket, buffer, MAX_DIM - 1);
                handle_error_rs(ret, c_socket);

                buffer[strcspn(buffer, "\n")] = 0;

                strncpy(unModBuff, buffer, 1024);

                singleSeatBuff = strtok(buffer, delim);			//Selecting only on seat

                strncpy(tempBuf, singleSeatBuff, strlen(singleSeatBuff)+1);

		        seatCheck = 0;

                puts("Before lock.\n");

                if (pthread_mutex_lock(list)) {
                    perror("[-]Mutex lock error");
                    pthread_exit(NULL);
                }

                puts("After lock.\n");

                while(singleSeatBuff) {					//As long as a seat exists in the reservation

                    strncpy(tempBuf, singleSeatBuff, strlen(singleSeatBuff)+1);

                    x = singleSeatBuff[0] - 65;
                    p1 = singleSeatBuff;
                    p2 = singleSeatBuff + 1;
                    if(memmove(p1, p2, strlen(singleSeatBuff)) == NULL) {	//Separating the row from the seat number
                        puts("[-]Memmove failure.\n");
                        pthread_exit(NULL);
                    }

                    y = atoi(singleSeatBuff);

                    curr = head;

                    while(curr) {					//Check if the seat has been already reserved by someone else

                        if(curr->s_i == x && curr->s_j == y) {
                            seatCheck = 1;
                            strcat(reservedSeats, " ");
                            strcat(reservedSeats, tempBuf);
                            break;
                        }
                        curr = curr->next;
                    }

                    singleSeatBuff = strtok(NULL, delim);		//Checking the next seat in the reservation

                }

                curr = head;

                if(seatCheck == 1) {
                    puts("[-]Reservation Failure (Some of the selected seats are already reserved)\n");
                    strcat(reservedSeats, "\n");
                    ret = safeWrite(c_socket, reservedSeats, strlen(reservedSeats));
                    handle_error_ws(ret, c_socket);
                    if (pthread_mutex_unlock(list)) {
                        perror("[-]Mutex unlock error");
                        pthread_exit(NULL);
                    }
                    goto reserv_routine;
                }
                else {
                    puts("[+]Check Done.... Starting to reserve the seats\n");
                    strcpy(buffer, "success\n");
                    ret = safeWrite(c_socket, buffer, strlen(buffer));
                    handle_error_ws(ret, c_socket);
                }

                singleSeatBuff = strtok(unModBuff, delim);

                code++;

                printList(head);

                for(int i=0; i<tokind; i++) {				//Insert reservation's seat in the list

                    s_node *new = malloc(sizeof(s_node));
                    if(!new) {
                        puts("[-]Malloc error.\n");
                        pthread_exit(NULL);
                    }

                    new->usr = malloc(strlen(username)+1);
                    new->usr=username;
                    new->s_i = singleSeatBuff[0] - 65;
                    p1 = singleSeatBuff;
                    p2 = singleSeatBuff + 1;
                    if(memmove(p1, p2, strlen(singleSeatBuff)) == NULL) {
                        puts("[-]Memmove failure.\n");
                        pthread_exit(NULL);
                    }
                    new->s_j = atoi(singleSeatBuff);
                    new->s_code = code;
                    new->next = head;
                    head = new;

                    fileB = fopen(filenameB, "a+");			
                    fprintf(fileB, "%d;%d;%d;%s\n", new->s_i, new->s_j, new->s_code, new->usr);	//Insert seat in the backup file
                    fclose(fileB);

                    singleSeatBuff = strtok(NULL, delim);
                }

                printList(head);

                n_counter += tokind;

		        sprintf(buffer, "%d\n", code);

                if (pthread_mutex_unlock(list)) {
                    perror("[-]Mutex unlock error");
                    pthread_exit(NULL);
                }

                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);
                break;
//
//CASE 3: DELETE A RESERVATION
//
            case 3:
                puts("CASE 3 T");
                ret = safeRead(c_socket, buffer, MAX_DIM - 1);
                handle_error_rs(ret, c_socket);

                buffer[strcspn(buffer, "\n")] = 0;

                int del_code = atoi(buffer);

                if (pthread_mutex_lock(list)) {
                    perror("[-]Mutex lock error");
                    pthread_exit(NULL);
                }

                int fdBD = open(filenameBD, O_RDWR | O_APPEND | O_CREAT, 0666);	//Creating a temporary backup file for safety reasons
                fileBD = fdopen(fdBD, "w+");
                if(!fileBD) {
                    perror("[-]Fdopen failure");
                    pthread_exit(NULL);
                }
		
                countV = 0;

                curr = head;

                printList(curr);

                while(curr->s_code == del_code && !strcmp(curr->usr, username)) {	//Checking if the reservation with said code exists and belongs to the user that requested the delete

                    s_node *del = curr;
                    sprintf(buffer, "%d;%d\n", curr->s_i, curr->s_j);
                    fprintf(fileBD, "%d;%d;%d;%s\n", curr->s_i, curr->s_j, curr->s_code, curr->usr);
                    head = curr->next;
                    curr = head;
                    free(del);
                    puts("[+]Node deleted.\n");

                    ret = safeWrite(c_socket, buffer, strlen(buffer));
                    handle_error_ws(ret, c_socket);
                    countV++;
                    n_counter--;
                }

                printList(curr);

                while(curr->next) {

                    while(curr->next->s_code == del_code && !strcmp(curr->next->usr, username)) {

                        s_node *del = curr->next;
			            sprintf(buffer, "%d;%d\n", del->s_i, del->s_j);
                        fprintf(fileBD, "%d;%d;%d;%s\n", del->s_i, del->s_j, del->s_code, del->usr);
                        curr->next = curr->next->next;
                        free(del);
                        puts("[+]Node deleted.\n");
                        countV++;
                        n_counter--;

                        ret = safeWrite(c_socket, buffer, strlen(buffer));
                        handle_error_ws(ret, c_socket);
                    }
                    curr=curr->next;
                }

                printList(curr);

                if(fclose(fileBD)) {
                    perror("[-]Error while closing the file");
                    pthread_exit(NULL);
                }

                if (pthread_mutex_unlock(list)) {
                    perror("[-]Mutex unlock error");
                    pthread_exit(NULL);
                }

                if(!countV) {
                    sprintf(buffer, "0\n");				//Sending ack to client if delete failed
                    ret = safeWrite(c_socket, buffer, strlen(buffer));
                    handle_error_ws(ret, c_socket);
                } else if (countV > 0) {
                    sprintf(buffer, "1\n");				//Sending ack to client if delete successful
                    ret = safeWrite(c_socket, buffer, strlen(buffer));
                    handle_error_ws(ret, c_socket);
                }

                break;

            /*case 4:
                puts("CASE 5 T\n");

                curr_n_seats = 0;

                curr = head;
                while(curr->s_j >= 0 && curr->s_i >= 0 && curr) {
                    printf("%d;%d\n", curr->s_i, curr->s_j);
                    if(!strcmp(curr->usr, username)) {
                        curr_n_seats++;
                        puts("strcmp done.\n");
                    }
                    curr = curr->next;
                }

                sprintf(buffer, "%d\n", curr_n_seats);
                ret = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_ws(ret, c_socket);

                if (curr_n_seats) {

                    curr = head;

                    while (curr->s_j >= 0 && curr->s_i >= 0 && curr) {
                        puts("inside while\n");
                        if (!strcmp(curr->usr, username)) {
                            sprintf(buffer, "%d;%d;%d\n", curr->s_i, curr->s_j, curr->s_code);
                            ret = safeWrite(c_socket, buffer, strlen(buffer));
                            handle_error_ws(ret, c_socket);
                        }
                        curr = curr->next;
                    }
                }

                break;*/
//
//CASE 4: EXIT
//
            case 5:
                printList(head);
                close(c_socket);
                printList(head);
                pthread_exit(NULL);
                break;




//
//CASE DEFAULT: INVALID INPUT
//
            default :
                printf("[-]Invalid input (= %d)\n", operation);
                pthread_exit(NULL);
        }

        ret = safeRead(c_socket, buffer, MAX_DIM - 1);
        handle_error_rs(ret, c_socket);

        buffer[strcspn(buffer, "\n")] = 0;

        if(!strcmp(buffer, "N"))
            break;
    }
    return 0;
}

int main() {

    int l_socket, c_socket, ret, b_choice;
    char *tokBuf;
    char backupBuf[1024];
    char backupBufC[1024];
    int fdBD;
    pthread_t tid;
    socklen_t addr_size;
    struct sockaddr_in server;
    struct sockaddr_in client;
    s_node *curr;

    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);			//Mask for ignoring all signals

    list = malloc(sizeof(pthread_mutex_t));			//Malloc for mutex list
    login = malloc(sizeof(pthread_mutex_t));			//Malloc for mutex login

    if(list == NULL || login == NULL) {
        puts("[-]Mutex malloc error.\n");
        exit(1);
    }

    if(pthread_mutex_init(list, NULL) || pthread_mutex_init(login, NULL)){				//Init and lock for both list and login
        puts("[-]Mutex init and lock error.\n");
        exit(1);
    }

    head = malloc(sizeof(s_node));				//Adding sentinel node to list
    if(!head) {
        puts("[-]Malloc1 Failure.\n");
        exit(1);
    }

    head->s_i = -1;
    head->s_j = -1;
    head->s_code = -1;
    head->usr = NULL;
    head->next = NULL;

    scanf_backup:
    puts("Do you want to load the data from a previous backup file?\n-) 1 to confirm\n-) 0 to create a new one\n");
    b_choice = 0;
    ret = scanf("%d", &b_choice);
    if(!ret) {
        puts("[-]Incompatible input type...\n");
        getc(stdin);
        goto scanf_backup;
    }
    if(ret == EOF) {
        puts("[-]Scanf failure...\n");
        pthread_exit(NULL);
    }

    if(b_choice == 1) {						//Server's user wants to load a backup file
        fileB = fopen(filenameB, "r+");
        if(!fileB) {
            puts("[-]Backup file might not exist, please create a new one.\n");
            goto scanf_backup;
        }

        fdBD = open(filenameBD, O_RDWR | O_CREAT | O_APPEND, 0666);	//Opening file to read datas
        fileBD = fdopen(fdBD, "w+");
        if(!fileBD) {
            perror("[-]Error while opening the backupDelete file");
            exit(1);
        }

        fgets(backupBuf, sizeof(backupBuf), fileB);		//Adding rows and seat number from the backup file
        backupBuf[strcspn(backupBuf, "\n")] = 0;
        tokBuf = strtok(backupBuf, ";");
        rows = atoi(tokBuf);
        tokBuf = strtok(NULL, ";");
        r_seats = atoi(tokBuf);

        while(fgets(backupBuf, sizeof(backupBuf), fileB)) {
            fseek(fileBD, 0, SEEK_SET);
            int match = 0;
            while(fgets(backupBufC, sizeof(backupBufC), fileBD)) {
                if (!strcmp(backupBuf, backupBufC)){
                    match = 1;
                    break;
                }
            }

            if(!match) {
                backupBuf[strcspn(backupBuf, "\n")] = 0;
                s_node *new = calloc(1, sizeof(s_node));
                if(!new) {
                    puts("[-]Calloc failure\n");
                    exit(1);
                }
                tokBuf = strtok(backupBuf, ";");
                new->s_i = atoi(tokBuf);
                tokBuf = strtok(NULL, ";");
                new->s_j = atoi(tokBuf);
                tokBuf = strtok(NULL, ";");
                new->s_code = atoi(tokBuf);
                tokBuf = strtok(NULL, ";");
                new->usr = malloc(128);
                strcpy(new->usr, tokBuf);
                new->next = head;
                head = new;
                n_counter++;
                printf("[+] Node loaded: i=%d | j=%d | code=%d | usr=%s\n", new->s_i, new->s_j, new->s_code, new->usr);
            }
        }

        truncate(filenameBD, 0);

        if (fclose(fileBD) || fclose(fileB)) {
            perror("[-]Error while closing the file");
            exit(1);
        }

        fileB = fopen(filenameB, "w+");
        if(!fileB) {
            perror("[-]Error while opening the backup file");
            exit(1);
        }

        fprintf(fileB, "%d;%d\n", rows, r_seats);

        curr = head;
        while(curr->s_code != -1) {
            puts("inside while\n");
            if(fprintf(fileB, "%d;%d;%d;%s\n", curr->s_i, curr->s_j, curr->s_code, curr->usr) != 7 + (int)strlen(curr->usr)) {
                perror("[-]Fprintf failure");
                pthread_exit(NULL);
            }
            curr = curr->next;
        }

        if (fclose(fileB)) {
            perror("[-]Error while closing the file");
            exit(1);
        }

	    code = 0;
        s_node *curr = head;
        while(curr->s_code != -1) {
            if(curr->s_code > code)
                code = curr->s_code;
            curr = curr->next;
        }


	puts("[+]Backup completed, starting the server...\n");
    }
    else if(b_choice == 0) {					//Server's user doesn't want to load a backup file
        puts("[+] Creating a new backup file...\n");
        fileB = fopen(filenameB, "w+");				//Creating a new backup file
        if(!fileB) {
            puts("[-] fopen failure (fileB)\n");
            exit(1);
        }

        fileBD = fopen(filenameBD, "w+");				//Creating a new backup file
        if(!fileBD) {
            puts("[-] fopen failure (fileBD)\n");
            exit(1);
        }

        fclose(fileBD);

        puts("[+] Creation completed, starting the server...\n");

        rows:
        puts("Welcome to the server program!\n\nHow many rows does your theater have?\n");
        ret = scanf("%d", &rows);
        if (!ret) {
            puts("[-]Incompatible type in scanf\n");
            getc(stdin);
            goto rows;
        }
        if (ret == EOF) {
            puts("[-]Scanf failure\n");
            exit(1);
        }

        seats:
        printf("How many seats per row does your theater have?\n");
        ret = scanf("%d", &r_seats);
        if (!ret) {
            puts("[-]Incompatible type in scanf\n");
            getc(stdin);
            goto seats;
        }
        if (ret == EOF) {
            puts("[-]Scanf failure\n");
            exit(1);
        }

        fprintf(fileB, "%d;%d\n", rows, r_seats);		//Adding rows and seat number to the backup file

        if (fclose(fileB) != 0) {
            puts("[-]Fclose failure (fileB)\n");
            exit(1);
        }
    }
    else {
        puts("[-]Invalid input, try again...");
        goto scanf_backup;
    }

    printf("[+]Total seats: %d\n", rows * r_seats);

    memset(&server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);				//Binding dinamically to port
    server.sin_addr.s_addr = htonl(INADDR_ANY);			//Binding dinamically to address

    l_socket = socket(AF_INET, SOCK_STREAM, 0);			//Socket creation
    if(l_socket < 0) {
        printf("[-]Error while connecting.\n");
        exit(1);
    }
    printf("[+]Server socket is created.\n");

    const int enable = 1;

    if (setsockopt(l_socket, SOL_SOCKET, SO_REUSEADDR , &enable, sizeof(int)) < 0)	//Option to reuse port
        exit(1);

    if(bind(l_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {		//Binding socket to port
        printf("[-]Port %d binding error.\n", PORT);
        exit(1);
    }
    printf("[+]Bind to port %d.\n", PORT);

    if(listen(l_socket, QUEUE) < 0) {							//Socket waiting for connections
        printf("[-]Listening error.\n");
        exit(1);
    }
    printf("[+]Listening...\n");


    while(1) {
        c_socket = accept(l_socket, (struct sockaddr *) &client, &addr_size);		//Accepting connection from client
        if(c_socket < 0) {
            printf("[-]Accept failure\n");
            exit(1);
        }
        printf("[+]Connection accepted from %s: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));


        if(pthread_create(&tid, NULL, thread_func, (void *)(long int)c_socket) != 0) {		//Creating threads
            puts("[-]Error during the creation of a thread\n");
            exit(1);
        }
        puts("[+]Thread successfully created\n");
    }
}
