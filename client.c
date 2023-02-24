#include "Client.h"
#include "Helper.h"

#define MAX_DIM 1024


int main(int argc, char **argv) {

    sigpipe.sa_handler = sigpipe_handler;
    sigint.sa_handler = sigint_handler;

    sigaction(SIGPIPE, &sigpipe, NULL);				//SIGPIPE handler
    sigaction(SIGINT, &sigint, NULL);				//SIGINT handler
    ParseCmdLine(argc, argv, &szAddress);

    c_socket = socket(AF_INET, SOCK_STREAM, 0);			//Socket creation
    if(c_socket < 0) {
        printf("[-]Error in connection.\n");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if( inet_aton(szAddress, &server.sin_addr) <=0 )		//Dynamic address
    {
        printf("Client: Non valid IP address.\n");

        if((he = gethostbyname(szAddress)) == NULL) {
            printf("GHBN fail.\n");
            exit(1);
        }
        printf("GHBN success.\n");
        server.sin_addr = *((struct in_addr *)he->h_addr);
    }

    if(connect(c_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {	//Connection to socket
        puts("[-]Error while connecting.\n");
        exit(1);
    }

    puts("Hi! Welcome to the theater site!\n");
    scanf_login:
    puts("Please select:\n\n1)Sign In\n\n2)Sign Up\n\n3)Exit\n\n\n");
    ret = scanf("%d", &choice);
    if(ret == 0) {
        puts("[-]Wrong input type.\n");
        getc(stdin);
        goto scanf_login;
    } else if (ret == EOF) {
        puts("[-]Scanf error.\n");
        exit(1);
    }

    sprintf(buffer, "%d\n", choice);

    switch(choice) {
//
//CASE 1: SIGN-IN
//
        case 1:
            puts("Case 1 LOGIN\n");
            rets = safeWrite(c_socket, buffer, strlen(buffer));
            handle_error_wc(rets, c_socket);

            printf("Buffer 1 :%s", buffer);

            puts("Please enter your username and password.\n\nPlease notice that all usernames must be unique and are case sensitive.\n");
        scanf_usr1:
            puts("Enter your username:\t");
            ret = scanf("%s", username);

            if(ret == 0) {
                puts("[-]Wrong input type.\n");
                goto scanf_usr1;
            } else if (ret == EOF) {
                puts("[-]Scanf error.\n");
                exit(1);
            }

        scanf_psw1:
            puts("Enter your password:\t");
            ret = scanf("%s", password);

            if(ret == 0) {
                puts("[-]Wrong input type.\n");
                getc(stdin);
                goto scanf_psw1;
            } else if (ret == EOF) {
                puts("[-]Scanf error.\n");
                exit(1);
            }

            sprintf(buffer, "%s;%s\n", username, password);

            rets = safeWrite(c_socket, buffer, strlen(buffer));
            handle_error_wc(rets, c_socket);


            rets = safeRead(c_socket, buffer, MAX_DIM - 1);
            handle_error_rc(rets, c_socket);

            buffer[strcspn(buffer, "\n")] = 0;

            if(!strcmp(buffer, "0")) {
                puts("[+]Sorry, username or password are not correct. Try again\nEnter your username:\t");
                goto scanf_login;				//If login creds were wrong, user can retry login or sign-up
            } else if(!strcmp(buffer, "1")) {
                puts("\n[+]You have been successfully logged in.\n");
            }
            break;
//
//CASE 2: SIGN-UP
//
        case 2:
            puts("CASE 2 LOGIN");
            rets = safeWrite(c_socket, buffer, strlen(buffer));
            handle_error_wc(rets, c_socket);

            puts("To register please enter your username and password.\n\nPlease notice that all usernames must be unique and are case sensitive.\n");
        scanf_usr2:
            puts("Enter your username (Max 20 characters):\t");

            ret = scanf("%s", username);

            if(ret == 0) {
                puts("[-]Wrong input type.\n");
                getc(stdin);
                goto scanf_usr2;
            } else if (ret == EOF) {
                puts("[-]Scanf error.\n");
                exit(1);
            }

            if(strlen(username) > 20) {
                puts("[-]The username must be at most 20 characters, retry please\n");
                getc(stdin);
                goto scanf_usr2;
            }

        scanf_psw2:
            puts("Enter your password (The password must be at least of 5 characters):\t");

            ret = scanf("%s", password);

            if(ret == 0) {
                puts("[-]Wrong input type.\n");
                getc(stdin);
                goto scanf_psw2;
            } else if (ret == EOF) {
                puts("[-]Scanf error.\n");
                exit(1);
            }

            if(strlen(password) < 5) {
                puts("[-]The password is too short, use at least 5 characters and retry please\n");
                goto scanf_psw2;
            }

        scanf_pswconf:
            puts("Enter you password again to confirm:\t");

            ret = scanf("%s", passwordC);

            if(ret == 0) {
                puts("[-]Wrong input type.\n");
                getc(stdin);
                goto scanf_pswconf;
            } else if (ret == EOF) {
                puts("[-]Scanf error.\n");
                exit(1);
            }

            if(strcmp(passwordC, password) != 0) {
                puts("[-]The passwords aren't the same, please try again.\n");
                getc(stdin);
                goto scanf_psw2;
            } else {
                sprintf(buffer, "%s;%s\n", username, password);
                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);
            }

            rets = safeRead(c_socket, buffer, MAX_DIM - 1);
            handle_error_rc(rets, c_socket);

            buffer[strcspn(buffer, "\n")] = 0;

            if(!strcmp(buffer, "0")) {
                puts("[-]Sorry, an account already exists with that username.\n");
                goto scanf_login;
            } else if(!strcmp(buffer, "1")) {
                puts("[+]The account has been created.\n");
            }
            break;
//
//CASE 3: EXIT
//
        case 3:
            puts("Hope you enjoyed your time here!\n");
            exit(0);

        default:
            puts("[-]Invalid input...\n");
            goto scanf_login;

    }


    while(1) {
        rets = safeRead(c_socket, buffer, MAX_DIM - 1);
        handle_error_rc(rets, c_socket);

        buffer[strcspn(buffer, "\n")] = 0;

        rows = atoi(strtok(buffer, delim));
        seats = atoi(strtok(NULL, delim));


        for(i = 0; i < rows; i++) {				//Creating matrix from datas received
            for(j = 0; j < seats; j++) {
                matrix[i][j] = (1+j)%(seats+1);
            }
        }

        scanf_retry:
        puts("Please select:\n\n1)Show map of the seats\n\n2)Buy Ticket\n\n3)Delete reservation\n\n4)Show all own reserved seats\n\n5)Exit\n");
        ret = scanf("%d", &choice);

        if(ret == 0) {
            puts("[-]Wrong input type.\n");
            getc(stdin);
            goto scanf_retry;
        } else if (ret == EOF) {
            puts("[-]Scanf error.\n");
            exit(1);
        }
        sprintf(buffer, "%d\n", choice);

        switch(choice) {
//
//CASE 1: PRINT THE MAP OF THE SEATS
//
            case 1:

                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);

                rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                handle_error_rc(rets, c_socket);

                buffer[strcspn(buffer, "\n")] = 0;

                reserved = atoi(buffer);

                for(i = 0; i < reserved; i++) {
                    rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                    handle_error_rc(rets, c_socket);

                    buffer[strcspn(buffer, "\n")] = 0;

                    r_row = atoi(strtok(buffer, delim));
                    r_seat = atoi(strtok(NULL, delim)) - 1;
                    matrix[r_row][r_seat] = -1; 		//Marking the element associated with the reserved seat with the value -1
                }

                for(i = 0; i < rows; i++) {			//Printing the map of the seats
                    puts("\n");
                    printf("%c\t", i+65);
                    for(j = 0; j < seats; j++) {
                        if (matrix[i][j] == -1) {
                            printf("RS  ");
                        } else {
                            printf("%02d  ", matrix[i][j]);
                        }
                    }
                }
                puts("\n");
                break;
//
//CASE 2: RESERVE A SEAT
//
            case 2:

                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);

            scanf_retry2:
                puts("Please enter the number of how many seats you want to reserve.\nPlease notice that the number given must be the same than the one on the next request.\n");
                int n_seats;
                ret = scanf("%d", &n_seats);

                if(ret == 0) {
                    puts("[-]Wrong input type.\n");
                    fflush(stdin);
                    goto scanf_retry2;
                } else if (ret == EOF) {
                    puts("[-]Scanf error.\n");
                    exit(1);
                }

                sprintf(buffer, "%d\n", n_seats);

                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);

                singleSeatBuff = calloc(1024, sizeof(char));
                if(!singleSeatBuff) {
                    puts("[-]Calloc failure\n");
                    exit(1);
                }

                fflush(stdin);
            scanf_retry_seats:

                puts("Please enter which seats you want to reserve.\nNOTE: format of selection must be for example A01.\n");

                if((fgets(buffer, MAX_DIM, stdin)) == NULL) {
                    puts("[-]Fgets error.\n");
                    exit(1);
                }

                strncpy(unModBuff, buffer, 1024);
                buffer[strcspn(buffer, "\n")] = 0;

                singleSeatBuff = strtok(buffer, delim1);
	
                while(singleSeatBuff) {				//Checking if seats are valid with all controls

                    int selectedRow = singleSeatBuff[0] - 65;

                    char *p1 = singleSeatBuff;
                    char *p2 = singleSeatBuff + 1;
                    if(memmove(p1, p2, strlen(singleSeatBuff)) == NULL) {
                        puts("[-]Memmove failure.\n");
                        exit(1);
                    }

                    if((selectedRow < 0 || selectedRow > rows) && (atoi(singleSeatBuff) < 0 || atoi(singleSeatBuff) > seats)) {
                        puts("[-]The numbers of rows and/or seats are invalid, please try again with valid values\n\n");
                        goto scanf_retry_seats;
                    }

                    if(selectedRow < 0 || selectedRow > rows) {
                        puts("[-]The number of row that you selected is invalid, please try again with a valid value\n\n");
                        goto scanf_retry_seats;
                    }

                    if(atoi(singleSeatBuff) < 0 || atoi(singleSeatBuff) > seats) {
                        puts("[-]The number of seat that you selected is invalid, please try again with a valid value\n\n");
                        goto scanf_retry_seats;
                    }

                    singleSeatBuff = strtok(NULL, delim1);
                }

                rets = safeWrite(c_socket, unModBuff, strlen(unModBuff));
                handle_error_wc(rets, c_socket);

                rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                handle_error_rc(rets, c_socket);

                buffer[strcspn(buffer, "\n")] = 0;

                if(!strcmp(buffer, "success")) {
                    puts("Seats reservation, wait a second...\n");
                    rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                    handle_error_rc(rets, c_socket);

                    buffer[strcspn(buffer, "\n")] = 0;

                    code = atoi(buffer);

                    strncpy(littleBuf, unModBuff, 512);
                    littleBuf[strcspn(littleBuf, "\n")] = 0;

                    sprintf(buffer, "[+]This is your reservation number for seat(s) %s:\t%04d\n", littleBuf, code);
                    scanf_retry3:
                    puts("Please enter a name for the file to save on your reservation code. NOTICE: name must be unique.\n");
                    ret = scanf("%s", filename);

                    if(ret == 0) {
                        puts("[-]Wrong input type...\n");
                        goto scanf_retry3;
                    } else if (ret == EOF) {
                        puts("[-]Scanf error.\n");
                        exit(1);
                    }

		    if(strcat(filename, ".txt") == NULL) {
			    puts("[-]Strncat error.\n");
			    exit(1);
		    }

                    fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0666);

                    if(fd == -1) {
                        puts("[-]Error while opening the file.\n");
                        goto scanf_retry3;
                    } else if (errno == EEXIST) {
                        puts("Already existing file name, please try with another one...\n");
                        goto scanf_retry3;
                    }

                    if(write(fd, buffer, strlen(buffer)) < 0) {
                        puts("[-]Error while writing on the file.\n");
                        exit(1);
                    }

                    if(close(fd) != 0) {
                        puts("[-]Error while closing the file.\n");
                        exit(1);
                    }
                }

                else{

                    printf("The seats %s are already reserved, please try again!\n", buffer);
                    goto scanf_retry2;
                }

                break;
//
//CASE 3: DELETE A RESERVATION
//
            case 3:

                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);

            scanf_code:
                puts("Please enter your code to delete your reservation.\n");

                ret = scanf("%s",buffer);

                if(ret == 0) {
                    puts("[-]Wrong input type.\n");
                    fflush(stdin);
                    goto scanf_code;
                } else if (ret == EOF) {
                    puts("[-]Scanf error.\n");
                    exit(1);
                }

                strcat(buffer, "\n");

                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);

                while(1) {

                    rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                    handle_error_rc(rets, c_socket);

                    buffer[strcspn(buffer, "\n")] = 0;

                    if(!strcmp(buffer, "1")) {
                        puts("Reservation successfully deleted.\n");
                        break;
                    } else if (!strcmp(buffer, "0")) {
                        puts("No reservation found with this code.\nNote: The code could have not belonged to you.\n");
                        break;
                    } else {
                        i = atoi(strtok(buffer, delim));
                        j = atoi(strtok(NULL, delim));
                        matrix[i][j-1] = j;			//Modify matrix
                    }
                }
                break;

            /*case 4:
                puts("CASE 5 T");
                strcpy(buffer, "4\n");
                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);

                rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                handle_error_rc(rets, c_socket);

                buffer[strcspn(buffer, "\n")] = 0;

                int curr_n_seats = atoi(buffer);

                if (!curr_n_seats) {
                    puts("No active reservations.\n");
                } else {
                    for (int i = 1; i <= curr_n_seats; i++) {
                        rets = safeRead(c_socket, buffer, MAX_DIM - 1);
                        handle_error_rc(rets, c_socket);

                        buffer[strcspn(buffer, "\n")] = 0;

                        resBuf = strtok(buffer, delim);
                        int curr_i = atoi(resBuf);
                        resBuf = strtok(NULL, delim);
                        int curr_j = atoi(resBuf);
                        resBuf = strtok(NULL, delim);
                        int curr_code = atoi(resBuf);

                        printf("Seat %d: %c%02d with code %d\n", i, (curr_i + 65), curr_j, curr_code);
                    }
                    puts("\n");
                }

                break;*/

//
//CASE 4: EXIT
//
            case 5:
                strcpy(buffer, "5\n");
                rets = safeWrite(c_socket, buffer, strlen(buffer));
                handle_error_wc(rets, c_socket);
                puts("Hope you enjoyed your time here!\n");
                exit(0);


//
//DEFAULT CASE: THE INPUT IS WRONG
//
            default:
                printf("opn : %d\n", choice);
                puts("Wrong input...\n");
                goto scanf_retry;
        }

        puts("Do you want to do something else? Type Y for yes or N for no\n");
        scanf_qc:
        fflush(stdin);
        ret = scanf("%s", buffer);
        if(ret == 0) {
            puts("Wrong input type.\n");
            goto scanf_qc;
        } else if (ret == EOF) {
            puts("[-]Scanf error.\n");
            exit(1);
        }

        if(!strcmp(buffer, "Y")) {
            strcat(buffer, "\n");
            rets = safeWrite(c_socket, buffer, strlen(buffer));
            handle_error_wc(rets, c_socket);
            goto scanf_retry;
        }
        else if(!strcmp(buffer, "N")) {
            strcat(buffer, "\n");
            rets = safeWrite(c_socket, buffer, strlen(buffer));
            handle_error_wc(rets, c_socket);
            puts("Thanks for using our site!\n");
            exit(1);
        }
        else {
            puts("Invalid input, valid ones: Y | N\n");
            goto scanf_qc;
        }
    }
}

int ParseCmdLine(int argc, char *argv[], char **szAddress) {

    int n = 1;

    while ( n < argc )
    {
        if ( !strncmp(argv[n], "-a", 2) || !strncmp(argv[n], "-A", 2) )
        {
            *szAddress = argv[++n];

        }
        else if ( !strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2) ) {

            printf("Syntax:\n\n");
            printf("    client -a (Remote address).\n\n");
            exit(EXIT_SUCCESS);
        }
        ++n;
    }
    if (argc==1)
    {
        printf("Syntax:\n\n");
        printf("    client -a (Remote address) -p (Remote port) [-h].\n\n");
        exit(EXIT_SUCCESS);
    }
    return 0;
}

void sigpipe_handler(int dummy) {
	write(STDOUT_FILENO, "Server N/A\n", 12);
	exit(1);
}

void sigint_handler(int dummy) {
	write(STDOUT_FILENO, "Exiting...\n", 12);
	exit(1);
}
