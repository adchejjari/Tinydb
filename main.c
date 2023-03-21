/*
 * Auteur : Adnane CHEJJARI
 * Matricule : 492759
 * Date : Decembre 2020
 * PROJET 2 - INFO-F201 : Systèmes d'exploitation
 * */
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "student.h"
#include "db.h"
#include "functions.h"
#include <pthread.h>
#include <signal.h>


int main(int argc, char* argv[]){
    signal(SIGINT, end_program); // Gestionnaire du signal
    printf("Welcome to the Tiny Database\n");
    if (argc>1){
    // Si l'utilisateur donne un fichier binaire à l'execution, copier le nom du fichier dans la variable binary_file_path
        strcpy(binary_file_path, argv[1]);
    }
    else {
    // Si l'utilisateur donne pas fichier db à l'execution,créer un nouveau fichier binaire et mettre son nom dans la variable binary_file_pat
        strcpy(binary_file_path,  "students_db.bin");
        FILE *newfile = fopen (binary_file_path, "wb");
        if (newfile==NULL){
            printf("fopen Failed\n");
            exit(EXIT_FAILURE);
        }
        fclose (newfile);
    }
    create_logfile();                                           // Creation d'un log directory
    db_init(&data);                                             // Initialisation de la db
    db_load(&data,binary_file_path);                            // Chargement de la base de donnée
    if (sem_init(&semaphore, 0, 1)!= 0){           // Initialisation du sémaphore
        printf("Semaphore initialisation Error");
        exit(EXIT_FAILURE);
    }
    char queries_to_execute[NUMBER_OF_QUERIES][QUERY_SIZE];     // Creation d'un tableau de string
    for(int j=0; j<NUMBER_OF_QUERIES; j++){strcpy(queries_to_execute[j],EMPTY_STR);} // Initialisation du tableau avec des string "vide"
    char query[QUERY_SIZE];                                       // Buffer
    if (isatty(0)==0){                                        // tester si un fichier est redirigé à l'execution
        multiple_exec(query, queries_to_execute);                 // Appel de la fonction qui analyse le fichier redirigé STDIN
        int numb_of_threads = 0;                                  // initialisation d'une variable contenant le nombre de threads
        compute_threads(queries_to_execute, &numb_of_threads);    // Calculer le nombre de threads
        pthread_t thread_liste[numb_of_threads];                  // Creation d'un tableau threads
        int i;                                                    // Compteur
        int pt;
        for(i=0; i<numb_of_threads; i++){
            printf("Running Query \' %s \' \n", queries_to_execute[i]);
            pt = pthread_create(&thread_liste[i], NULL ,execute_request, queries_to_execute[i]); // Creation des threads
            if (pt != 0){                                                                             // Gestion d'erreur
                printf("Error creating thread\n");
                exit(EXIT_FAILURE);
            }
        }
        int j;
        int jn;
        for(j=0; j<numb_of_threads;j++){
            jn = pthread_join(thread_liste[j], NULL); // Joindre les threads
            if (jn != 0){                                         // Gestion d'erreur
                printf("Joining error\n");
                exit(EXIT_FAILURE);
            }
        }
        db_save(&data, binary_file_path);                         // Sauvegarder la base de données dans le fichier binary_file_path
    }
    else{
    //Si pas de fichier redirigé lors de l'execution du programme
        single_exec();                                            // appel de la fonction qui execute une requete
    }
    if(sem_destroy(&semaphore)!= 0){                              // Detruire le sémaphore
        exit(EXIT_FAILURE);
    }
    return 0;
}

