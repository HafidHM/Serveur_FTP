#include "csapp.h"

#define MAX_NAME_LEN 256
#define SIGCHILD 20
#define NPROC  3
pid_t pid;
int status;
void echo(int connfd);

    
void handler_CHILD(int sig){    //traitant permet dattendre tous les process fils du processus pere 
    pid_t pid;
    if ((pid = waitpid(-1, NULL, 0)) < 0)
        unix_error("waitpid error");
    printf("Handler reaped child %d\n", (int)pid);
    Sleep(2); /* the only purpose of this line is to exacerbate the problems */
    return;
}
void handler_SIGINT(int sig){ // Ctrl+C
   printf("J'ai recus un SIGINT\n");
   kill(0,sig);
   exit(0);
}

int main(int argc, char **argv){

   
    int listenfd, connfd, port;
    Signal(SIGCHLD, handler_CHILD);
    Signal(SIGINT, handler_SIGINT);
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN], client_hostname[MAX_NAME_LEN];
    
    
    port = 2121;  //Port par default du Serveur FTP
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port); // liste d'attente
   
for(int i=0 ; i<NPROC ; i++){   //Boucle pour cree a l'avance des processus (methode pool)
  pid =fork();
  if(pid==0){
    while (1) {
        // connfd est une flux d'entre sortie descrip read write
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        

        /* determine the name of the client */
        Getnameinfo((SA *) &clientaddr, clientlen,
                    client_hostname, MAX_NAME_LEN, 0, 0, 0);
        
        /* determine the textual representation of the client's IP address */
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                  INET_ADDRSTRLEN);
        
        printf("server connected to %s (%s)\n", client_hostname,
               client_ip_string);

        echo(connfd);    //Executer une commande commande envoyee par un Client 
        Close(connfd); //Fermer la connection avec les Clients 
       }  
    }

 }  // le processue père qui attends la fin du fils
 for(int i=0; i<NPROC; i++ ){
    wait(NULL);
 }
 Close(listenfd);
}
