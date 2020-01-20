/*
 * echo - read and echo text lines until client closes connection
 */
#include <stdio.h>
#include <stdlib.h> 
 #include <sys/types.h> 
#include <dirent.h> 
#include <unistd.h> 
#include "csapp.h"
#define max_argv 3   //nombre max d'arguments envoyee par le Client
#define MAX 500    //taille d'un paquet d'octets



int size(int fd){  // renvoyer la taille du fichier en octet dans le Serveur
    long int size=0 ;
    struct stat info;
    fstat(fd, &info);
    size=(int)info.st_size;
    return size;
}

void echo(int connfd)
{   
    int  size_fichier,i=0, nb_pq,x,fd, size_rcp, size_rest=0,n , nomfich;
    char buf[MAXLINE], mem[MAX],argv1[max_argv][MAXLINE],recup[MAX], *mot;
    rio_t rio;


    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);          //Recevoire un buffer dans lequel y a les commandes a executees 
     if (strcmp(buf, "ls")!=0||strcmp(buf, "pwd")!=0){   //tester si on a qu'une seul commande dans le buffer
            mot = strtok(buf, " ");          
    
             while( mot != NULL && i<max_argv )  //Boucle pour diviser le buffer et extraire la commande apres les maitre dans un matrice 
             {
              strcpy(argv1[i], mot);
              
              i++;
              mot = strtok(NULL, " ");
             }
               nomfich = strlen(argv1[1]);
                argv1[1][nomfich-1]='\0';
     }
	

   

    if(strcmp(argv1[0], "get")==0){   //tester si on a une commande de type "GET"

                fd = Open(argv1[1], O_RDONLY, 0);         //Ouvrire le fichier demandee
                if (fd < 0)
                {
                	printf("erreur ouvertur fichier \n");
                }
            

             size_rcp = atoi(argv1[2]);             //Extraire la taille du fichier sil existe deja dans le Client si oui donc la taille sera !=0

             size_fichier = size(fd);                 //Calcule la taille du fichier
           
             sprintf(recup, " %d", size_fichier);        // Enregistere la taille du fichier dans un buffer
             Rio_writen(connfd, recup, strlen(recup));  // l'envoie de la taille du fichier au client

             if(size_rcp){
                printf("download du fichier %s  a partir du %d eme octet\n", argv1[1], size_rcp );
                lseek(fd, size_rcp-1, SEEK_SET);         //Se deplacer vers le  Offset pour etre daccord avec le fichier dans le client, 
             }                                            // ca c est pour gerer les pannes 


             printf("telechargement de  %d octet de %s\n", size_rest, argv1[1] );
             size_rest = size_fichier -  size_rcp ;
            
            // envoie le fichier au client
            nb_pq=  size_rest/MAX;                 //Calcule le nbre des paquets de taille MAX a envoye
            int restpaq  = size_rest - (nb_pq*MAX);
             Rio_writen(connfd, mem, MAX);      //Envoyer le premier paquet
            for (i = 0; i<nb_pq; i++){   //L'envoie des autres paquets
                sleep(1);       //pour bien visualiser le telechargement 
                memset(mem, 0, MAX);
                n=Rio_readn(fd, mem, MAX);         // lire 15 octets du fichier
                Rio_writen(connfd, mem, n);           //et les envoyees au Client 
            }


            // Envoyer  le reste des paquet non envoyer
            if(restpaq !=0){
               memset(mem, 0, MAX);
               if(( x =Rio_readn(fd, mem, restpaq))>0){
                    Rio_writen(connfd, mem, x);
               }
            }
  }else if(strcmp(buf, "pwd")==0){   //tester si la commande pwd 
        char tampon[MAXLINE];
        getcwd(tampon, MAXLINE);        //stocker le resulat de la fonction getcwd dans un tampon
        tampon[strlen(tampon)] = '\n';    // Enlever le "\n"
        Rio_writen(connfd, tampon , MAXLINE);   //Envoyer le tampon au Client 

  }else if(strcmp(buf, "ls")==0){  //tester si la commande pwd 

            struct dirent *lecture;
            DIR *rep;
            char list_rep[MAXLINE]; 

            char recup1[MAXLINE];
            memset(list_rep, 0 , MAXLINE);
            rep = opendir("." );  //Ouverire un tube  pour lister le contenu du repertoire courant
            while ((lecture = readdir(rep))) {  //Extraire les noms des fihiers existant dans le repertoire courant 
                sprintf(recup1, " %s", lecture->d_name);         
                strcat(list_rep , recup1);     //les  enregistes dans un buffer list_rep 
            }
            list_rep[strlen(list_rep)]='\n';

            Rio_writen(connfd, list_rep, MAXLINE);        //Envoyer le list_rep au Client 
            closedir(rep);   //Fermer le tube  


  } else if(strcmp(buf, "mkdir")==0){
    int check;
    char recup1[MAXLINE]; 
    char* nomD = argv1[1]; 
    check = mkdir(nomD, 0777 ); 
    sprintf(recup1, " %d", check);
    write(connfd, recup1 , strlen(argv1[1]));
     }

         

}

