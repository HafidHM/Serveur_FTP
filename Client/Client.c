/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max_argv 2   //nombre max d'arguments tapee par le Client
#define MAX 500    //taille d'un paquet d'octets


int size_file(int fd){  // fonction renvoie la taille du fichier en octet
    long int size=0 ;
    struct stat info;
    fstat(fd, &info);
    size=(int)info.st_size;
    return size;
}


void get_file(int clientfd,char* cmd,rio_t* rio,char buf[MAXLINE]){    // fonction permet de telecharger un fichier depuis le serveur FTP
    int size_fichier, nb_pq,fd,size_actu=0, size_rest,x,n;
    char mem[MAX], size[MAX];
    struct timeval tps_av, tps_ap;
    struct timezone tz;
    float a;
   

         fd = Open(cmd, O_CREAT | O_APPEND | O_RDWR , 0666);     //Ouvrire un fichier du meme nom
        if(fd<0){
            printf("Erreur d'ouverture du fichier \n");
            exit(0);
        }

        size_actu = size_file(fd);
         char offset[4];
         char fichier[9];
         char buf1[MAXLINE];
         sprintf(offset, " %d", size_actu);               //enregistrer l'Offset dans le buffer offset
         sprintf(fichier, " %s", cmd);                    //meme chose pour le nom du fichier

        strcat(buf1,"get");
        strcat(buf1,fichier);
        strcat(buf1,offset);
        strcpy(buf,buf1);              //Concatener la commande get + nom fichier +  Offset dans un buffer pour les envoyees au serveur

       // Rio_writen(clientfd, buf, MAXLINE);
        // recuperer la taille du fichier envoyer par le serveur
        if (Rio_readlineb(rio, size, MAX) > 0) {
            size_fichier = atoi(size);             //Convertir la taille du fichier recus depuis le serveur convertir en size_t
        } 

         printf("télechargement du fichier %s de taille %d / %d\n",cmd ,size_actu, size_fichier);
        size_actu = size_file(fd);     //Recuperer size actuel du fichier (L'Offset)

        if(size_fichier == size_actu){
            printf("fichier deja existant\n");
            Close(fd);
            exit(0);
        }
        else { 
            printf("Debut de telechargement d'un fichier de taille: %d\n", size_fichier);
            if (size_actu !=0) {//Si le fichier  n'est pas vide
                lseek(fd, size_actu-1, SEEK_SET);    //se deplacer dans le fichier precisement apartir de l'Offset
            }


         // chargement par paquet de 15 octets
          gettimeofday (&tps_av, &tz); // Debut de télechargement
           size_rest = size_fichier - size_actu;     //Calcule du nombre doctets restants a recevoire
           nb_pq = size_rest/MAX;                          //diviser la taille restante en paquets de MAX octets
           int reste = size_rest - (nb_pq*MAX);  // la taille restante
            memset(mem, 0, MAX);             // alouer la memoire pour mem
            for (int i = 0; i<nb_pq; i++){         
                if((x=Rio_readn(clientfd, mem, MAX))>0){
                    write(fd , mem, x);          //ecrire les paquets recus dans un fichier dans le client
                    size_actu = size_file(fd);
                    printf("transfers en cours de %d paquets ...(%d / %d)\n", nb_pq,size_actu, size_fichier );
                }
            }
            if(reste != 0){     //recevoire le reste des paquets inferieure a MAX
                memset(mem, 0, MAX);
                if(( n =Rio_readn(clientfd, mem, reste))>0){
                    write(fd , mem, n);
                    size_actu = size_file(fd);
                    printf("transfers en cours du reste des %d Octets. (%d / %d)\n", n ,size_actu, size_fichier );
                }
            }    
            close(fd);  //fermer le descripteur du fichier
        }

        gettimeofday (&tps_ap, &tz);//Fin de télechargement
        long long diff =(tps_ap.tv_sec - tps_av.tv_sec) * 1000000L + ( tps_ap.tv_usec - tps_av.tv_usec); // Calcule du temps du transfert
        n= size_rest;
        printf(" tranfer successfully complete\n");
         a = n*0.001/diff*0.000001;   // Calcule de la vitesse du transfert
        printf("%d bytes received in %llu seconds (%f kbytes/s)\n", size_actu , diff, a);


}




int main(int argc, char **argv)
{
    int clientfd,port;
    char *host, buf[MAXLINE],input[MAXLINE], *cmd;
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
        exit(0);
    }

    host = argv[1]; 
    port = 2121; 

    clientfd = Open_clientfd(host, port);

    printf("client connected to server OS\n");
    printf("ftp>"); 
    
    Rio_readinitb(&rio, clientfd);

    // lecture entrée standard et copier dans buf
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
      // Exraire  la commande 
    	Rio_writen(clientfd, buf, strlen(buf)); 

        memcpy(input,buf,strlen(buf)-1);
        cmd = strtok(input, " ");         //Extraire la commande tapee par le client


        if (strcmp(cmd,"get")==0){
            // Exraire le nom du fichier
            cmd = strtok(NULL, " ");   //Extraire le nom du fichier tapee par le client
            get_file(clientfd,cmd,&rio,buf);   // chercher un fichier dans le serveur pour le telecharager

        } 
        else if(strcmp(cmd, "pwd")==0||strcmp(cmd, "ls")==0){    

            Rio_writen(clientfd, cmd, MAXLINE);   //l'envoie du buffer ou  il y a la commande soit "ls" ou "pwd".
            char tampon[MAXLINE];
            int n;
            if ( (n = Rio_readlineb(&rio, tampon, MAXLINE)) > 0){ //attendre  le resultat de la commande "ls" ou "pwd"
                    tampon[n-1]='\0';   //enlever le caractere "\n"
                    printf("%s\n", tampon);    //affichage du resultat
            }
        }   
        else if(strcmp(cmd , "mkdir")==0){
        char check[ MAXLINE];
        int nv;
         Rio_readlineb(&rio, check, MAXLINE) ;
          nv = atoi(check);
      if ( nv == 0) 
        printf("Directory created\n"); 
      else { 
        printf("Unable to create directory\n"); 
        exit(1); 
       } 

     }   

        else{
            printf("Cette commande n'existe pas\n");
        }
  
        printf("ftp>"); 
    }
 
}

