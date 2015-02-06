/*
 * File: server.c created by Jorge Duran (aka ganchito55)
 *
 **/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define PORT 5555

	int FIN = 0;

void serverTCP(int s, struct sockaddr_in clientaddr_in);
void createHTMLCode(char *out);
void replaceVar(char *in,char *out);

void finalizar()
{
    FIN = 1;
}

int main(int argc,char * argv[]){

	int s_Handler; /*connected socket descriptor*/
	int ls_Handler; /* listen socket descriptor */

	struct sockaddr_in myAddr; /* local socket address */
	struct sockaddr_in clientaddr_in; /* client socket address */
	struct sigaction vec;

	struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */

	int FIN = 0;
    int numfds;
	int addrlen;
	addrlen = sizeof (struct sockaddr_in);
    fd_set readmask;

	/* clear address structure */
	memset((char *)&myAddr,0,sizeof(struct sockaddr_in));
	memset((char *) &clientaddr_in, 0, sizeof (struct sockaddr_in));

    /* Set up address structure for the listen socket. */
    myAddr.sin_family = AF_INET;
    /* The server should listen on the wildcard address,
     * rather than its own internet address.  This is
     * generally good practice for servers, because on
     * systems which are connected to more than one
     * network at once will be able to have one server
     * listening on all networks at once.  Even when the
     * host is connected to only one network, this is good
     * practice, because it makes the server program more
     * portable.
     */
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(PORT);

	/* Create the listen socket. */
    ls_Handler = socket(AF_INET, SOCK_STREAM, 0);
    if (ls_Handler == -1)
    {
        perror(argv[0]);
        fprintf(stderr, "%s: unable to create socket TCP\n", argv[0]);
        exit(1);
    }
    /* Bind the listen address to the socket. */
    if (bind(ls_Handler, (const struct sockaddr *) &myAddr, sizeof (struct sockaddr_in)) == -1)
    {
        perror(argv[0]);
        fprintf(stderr, "%s: unable to bind address TCP\n", argv[0]);
        exit(1);
    }
    /* Initiate the listen on the socket so remote users
     * can connect.  The listen backlog is set to 5, which
     * is the largest currently supported.
     */
    if (listen(ls_Handler, 5) == -1)
    {
        perror(argv[0]);
        fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
        exit(1);
    }

	/* Now, all the initialization of the server is
     * complete, and any user errors will have already
     * been detected.  Now we can fork the daemon and
     * return to the user.  We need to do a setpgrp
     * so that the daemon will no longer be associated
     * with the user's control terminal.  This is done
     * before the fork, so that the child will not be
     * a process group leader.  Otherwise, if the child
     * were to open a terminal, it would become associated
     * with that terminal as its control terminal.  It is
     * always best for the parent to do the setpgrp.
     */
    setpgrp();

    switch (fork())
    {
    case -1: /* Unable to fork, for some reason. */
        perror(argv[0]);
        fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
        exit(1);

    case 0: /* The child process (daemon) comes here. */

        /* Close stdin and stderr so that they will not
         * be kept open.  Stdout is assumed to have been
         * redirected to some logging file, or /dev/null.
         * From now on, the daemon will not report any
         * error messages.  This daemon will loop forever,
         * waiting for connections and forking a child
         * server to handle each one.
         */
        close(stdin);
        close(stderr);

        /* Set SIGCLD to SIG_IGN, in order to prevent
         * the accumulation of zombies as each child
         * terminates.  This means the daemon does not
         * have to make wait calls to clean them up.
         */
        if (sigaction(SIGCHLD, &sa, NULL) == -1)
        {
            perror(" sigaction(SIGCHLD)");
            fprintf(stderr, "%s: unable to register the SIGCHLD signal\n", argv[0]);
            exit(1);
        }

        /* Register SIGTERM  */
        vec.sa_handler = (void *) finalizar;
        vec.sa_flags = 0;
        if (sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1)
        {
            perror(" sigaction(SIGTERM)");
            fprintf(stderr, "%s: unable to register the SIGTERM signal\n", argv[0]);
            exit(1);
        }

 		while (!FIN)
        {
            FD_ZERO(&readmask);
            FD_SET(ls_Handler, &readmask);       

            if ((numfds = select(getdtablesize(), &readmask, (fd_set *) 0, (fd_set *) 0, NULL)) < 0)
            {
                if (errno == EINTR)
                {
                    FIN = 1;
                    perror("\nselect failed\n ");
                }
            }
            else
            {  
                if (FD_ISSET(ls_Handler, &readmask))
                {
                    /* Note that addrlen is passed as a pointer
                     * so that the accept call can return the
                     * size of the returned address.
                     */
                    /* This call will block until a new
                     * connection arrives.  Then, it will
                     * return the address of the connecting
                     * peer, and a new socket descriptor, s,
                     * for that connection.
                     */
                    s_Handler = accept(ls_Handler, (struct sockaddr *) &clientaddr_in, &addrlen);
                    if (s_Handler == -1) exit(1);
                    switch (fork())
                    {
                    case -1: /* Can't fork, just exit. */
                        exit(1);
                    case 0: /* Child process comes here. */
                        close(ls_Handler); /* Close the listen socket inherited from the daemon. */
                        serverTCP(s_Handler, clientaddr_in);
                        exit(0);
                    default: /* Daemon process comes here. */
                        /* The daemon needs to remember
                         * to close the new accept socket
                         * after forking the child.  This
                         * prevents the daemon from running
                         * out of file descriptor space.  It
                         * also means that when the server
                         * closes the socket, that it will
                         * allow the socket to be destroyed
                         * since it will be the last close.
                         */
                        close(s_Handler);
                    }
                } 
 			}
        } 
        close(ls_Handler);
        printf("\nFin de programa servidor!\n");
    default: /* Parent process comes here. */
        exit(0);
    }
}

void serverTCP(int s, struct sockaddr_in clientaddr_in)
{
	int len;
	char buffer[65536],sendBuffer[65536];
	len = recv(s, buffer, 65536, 0); /*Recive explorer info */
      	if(len == -1) perror;
        if (len == 0) return;     
	
	memset(buffer,'\0',sizeof(buffer)); // Clear buffer
	createHTMLCode(buffer);	
	sprintf(sendBuffer,"HTTP/1.1 200 OK\nServer: RPmonitor\nContent-Type: text/html; charset=utf-8\nConnection: close\nContent-Length: %d\n\n\n\n%s",(int) strlen (buffer),buffer);
	//printf("\n%s\n",sendBuffer);

	if (send(s, sendBuffer, strlen(sendBuffer), 0) != strlen(sendBuffer)) fprintf(stderr,"Error when sending the html code");
	
}


/****************************************************************************/
/* CreateHTMLCode read the file monitor.html and load in a char array       */
/****************************************************************************/
void createHTMLCode(char *out){
 
	int i,j=0,flag=0,flagVar=0,z=0;         //index i control the HTML pos index j control de array with the msg to send
	FILE *f;
	char buffer[500],vars[100];	//buffer for each line

	if((f=fopen("monitor.html","r"))==NULL){
		fprintf(stderr,"Error open the html source");
		exit(1);
	}
	
	fscanf(f,"%[^\n]%*c",buffer); //read line and flush the final \n	
	while(!feof(f)){
		flag=0;    	//Control spaces for minify
		flagVar=0; 	//Replace vars for their resolves
		z=0;		//Control de resolve string
		for(i=0;i<strlen(buffer);i++){   //iterate in the line of HTML code
			if((buffer[i]=='\t') || ((buffer[i]==' ') && (flag==0))){ //minify
				continue;
			}
			if(buffer[i] == '"'){  //C version of " character
				out[j]='\"';
				j++;
				continue;
			}
			if(buffer[i] == '$' || flagVar==1){ //search and replace vars example $temp$ 55ºC
				vars[z]=buffer[i];
				z++;
				if(flagVar==0 && buffer[i] == '$'){//start 
					flagVar=1;
					continue;
				}
				if(flagVar==1 && buffer[i] == '$'){//end
					flagVar=0;
					vars[z]='\0';
					replaceVar(vars,vars); 
					for(z=0;z<strlen(vars);z++){
						out[j]=vars[z];
						j++;
					}
					z=0; // for two o more vars in a same line example $temp$  / $load$	
				}
				continue;
			}			
			out[j]=buffer[i];
			j++;
			flag=1; //Not remove internal spaces ' ' example <meta charset> and not <metacharset>
		}		
		fscanf(f,"%[^\n]%*c",buffer);
	}

	fclose(f);
}

/*****************************************************************************/
/* ReplaceVar replace var with the output of the run command                 */
/* Example: input $temp$ run /opt/vc/bin/vcgencmd measure_temp  and          */  
/* write in output 50ºC see file vars and Readme.md for more info            */
/*****************************************************************************/       
void replaceVar(char *in,char *out){
	char vars[100],command[100];
	FILE *file;
	FILE *f;	
	if((f=fopen("var","r"))==NULL){
		fprintf(stderr,"Error when open var files");
	}

	fscanf(f,"%[^\t]%*c%[^\n]%*c",vars,command);
	while(!feof(f)){
		if(strcmp(in,vars)==0){
			file = popen(command,"r");
			fscanf(file,"%[^\n]%*c",out);
			pclose(file);			
			break;
		}
		fscanf(f,"%[^\t]%*c%[^\n]%*c",vars,command);
	}	
	fclose(f);
}
