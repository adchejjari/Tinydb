/*
 * Auteur : Adnane CHEJJARI
 * Matricule : 492759
 * Date : Decembre 2020
 * PROJET 2 - INFO-F201 : Systèmes d'exploitation
 * */
#define __USE_XOPEN
#define _GNU_SOURCE
#define NUMBER_OF_QUERIES 200
#define EMPTY_STR " "
#define QUERY_SIZE 256
#define BUFFER_SIZE 256
#include <stdio.h>
#include "student.h"
#include <stdlib.h>
#include "db.h"
#include <semaphore.h>

database_t data;                    // base de données
sem_t semaphore;                    // Sémaphore pour assurer une exclusion mutuelle
char binary_file_path[BUFFER_SIZE]; // nom du Fichier binaire



void getfilename(char* query, char *tstmp);
/*
 * Fonction de création du nom du fichier log
 * parametres :
 * query = type de la requete (select/delete/update/insert)
 * tstmp = buffer qui stock le nom du fichier log
 */


void update_student(char* field, char*value, student_t* student, database_t* db);
/*
 * Fonction qui met à jour les données d'un étudiant
 * parametres :
 * field = le champs de la données à mettre à jour (fname/lname/id/section/birthdate)
 * value = la donnée en question
 * student = l'étudiant à mettre à jour
 * db = base de donnée
 */

void insert_student(char* fname, char* lname, unsigned *id, char* section, struct tm* birthdate);
/*
 * Fonction qui insert un étudiant dans une base de donnée
 * parametres :
 * fname = prenom de l'étudiant
 * lname = nom de famille de létudiant
 * id = id de l'étudiant
 * section = section de l'étudiant
 * birthdate = date de naissance de l'étudiant
 * db = base de donnée où l'étudiant doit être inséré
 */

void write_db_file(database_t* db, FILE* file);
/*
 * Fonction permet d'écrire dans le fichier file les données de la database db
 * parametres :
 * db = base de donnée qui contien les données à écrire
 * file = fichier dans lequel il faut ecrire les données
 */

void select_rq(database_t *db, char* request,char* filter, char* value, char* filtre_update, char* value_update, char* req_write);
/*
 * Fonction qui selectionne un étudiant dans une base données.
 * Cette fonction peut supprimer ou mettre à jour les données d'un étudiant selon la requete
 * parametres :
 * db = la base de donnée
 * request = type de la requete ex: delete, select, update
 * filtre = le champs de la données à selectionner (fname/lname/id/section/birthdate)
 * value = la valeur de la selection
 * filtre_update = le champs à actualiser (en cas d'update)
 * value_update = nouvelle valeur
 */

void create_logfile();
/*
 * Création du repertoire log
 */

void* execute_request(void* query);
/*
 * Cette fonction execute une requete en faisant appel aux autres fonction de parsing (parsing.c)
 * parametres :
 * query = la string contenant la requette a executer
 * database = la base de donnée concernée par la requete
 */

void multiple_exec(char* request, char liste[NUMBER_OF_QUERIES][QUERY_SIZE]);
/*
 * fonction qui prend les requete d'un fichier < et mets les mets dans un array
 * Parametre :
 * request = buffer
 * liste = Liste 2D vide
 * numb_queries = entier qui nombre de requete trouvée dans le fichier
 */

void single_exec();
/*
 * Fonction appelée pour traiter une requete utilisateur saisie en ligne de commande
 */

void compute_threads(char liste[NUMBER_OF_QUERIES][QUERY_SIZE], int* number_of_threads);
/*
 * Compte le nombre de requête dans la liste
 */

void end_program();
/*
 * fonction qui met fin au programme lors d'un sigint (signal handler)
 */

