/*
 * Auteur : Adnane CHEJJARI
 * Matricule : 492759
 * Date : Decembre 2020
 * PROJET 2 - INFO-F201 : Systèmes d'exploitation
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "functions.h"
#include "parsing.h"
#include <time.h>

void end_program() {
    printf("\nWaiting for requests to terminate..\n");
    db_save(&data, binary_file_path);
    exit(0);
}

void getfilename(char* query, char *tstmp){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    long start_ns = now.tv_nsec + 1e9 * now.tv_sec;
    sprintf(tstmp, "logs/%lu-%s.txt",start_ns, query );
}

void update_student(char* field, char*value, student_t* student, database_t* temp){
    if (strcmp(field, "id")==0){student->id = atoi(value);}
    else if (strcmp(field, "fname")==0){strcpy(student->fname, value);}
    else if (strcmp(field, "lname")==0){strcpy(student->lname, value);}
    else if (strcmp(field, "section")==0){strcpy(student->section, value);}
    else if(strcpy(field, "birthdate")==0){strptime(value, "%d/%m/%Y", &student->birthdate);}
    db_add(temp, *student);
}

void insert_student(char* fname, char* lname, unsigned *id, char* section, struct tm* birthdate){
    clock_t start = clock();
    FILE * file;
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    getfilename("insert", filename);
    file = fopen(filename, "a");
    if (file==NULL){
        printf("fopen failed\n");
        exit(EXIT_FAILURE);}
    student_t* student = malloc(sizeof(student_t));
    strcpy(student->fname, fname); strcpy(student->lname, lname); strcpy(student->section, section);
    student->id = *id; student->birthdate = *birthdate;
    if (sem_wait(&semaphore)!= 0){                         /** decrementer la valeur de la sémaphore**/
        exit(EXIT_FAILURE);
    }
    db_add(&data, *student);                               /** SECTION CRITIQUE **/
    if (sem_post(&semaphore)!= 0){                         /** incrementer la valeur de la sémaphore**/
        exit(EXIT_FAILURE);
    }
    clock_t end = clock();
    double execution_time = (double)(end-start)/CLOCKS_PER_SEC *1000;
    fprintf(file,"Query \"insert %s %s %u %s %d/%d/%d\" Completed in %lf ms with 1 results\n", fname, lname, *id, section, birthdate->tm_mday, birthdate->tm_mon+1, birthdate->tm_year+1900, execution_time);
    student_to_str(buffer,student);
    if (isatty(0)!=0) printf("%s\n", filename);
    fprintf(file, "%s",buffer);
    fclose(file);
}

void write_db_file(database_t* db, FILE* file){
    int i = 0;
    while(i < db->lsize){
        char buffer[BUFFER_SIZE];
        student_to_str(buffer, &db->data[i]);
        fprintf(file,"%s\n", buffer);
        i++;
    }
}

void select_rq(database_t *db, char* request,char* filter, char* value, char* filtre_update, char* value_update, char* query_write) {
    clock_t start = clock();
    database_t temporary ;
    db_init(&temporary);
    char filename[200];
    getfilename(request, filename);
    FILE * file;
    file = fopen(filename, "a");
    if (file==NULL){
        printf("fopen failed\n");
        exit(EXIT_FAILURE);}
    int i = 0;
    while (i < db->lsize) {
        char search[50];
        if(strcmp(filter,"fname")==0){strcpy(search, db->data[i].fname);}
        else if (strcmp(filter,"lname")==0){strcpy(search, db->data[i].lname);}
        else if (strcmp(filter,"section")==0){strcpy(search, db->data[i].section);}
        else if (strcmp(filter,"id")==0){sprintf(search,"%d", db->data[i].id);}
        else if (strcmp(filter, "birthdate")==0){sprintf(search, "%d/%d/%d", db->data[i].birthdate.tm_mday, db->data[i].birthdate.tm_mon+1, db->data[i].birthdate.tm_year+1900);}
        if (strcmp(search, value)==0){
            if (strcmp(request,"select")==0){
                db_add(&temporary, db->data[i]);
            }
            if (strcmp(request, "delete")==0){
                db_add(&temporary, db->data[i]);
                if (sem_wait(&semaphore)!= 0){                         /** decrementer la valeur de la sémaphore**/
                    exit(EXIT_FAILURE);
                }
                db_delete(db, &(db->data[i]));                         /**Section critique**/
                if (sem_post(&semaphore)!= 0){                         /** incrementer la valeur de la sémaphore**/
                    exit(EXIT_FAILURE);
                }
            }
            else if (strcmp(request, "update")==0){
                if (sem_wait(&semaphore)!= 0){                                         /** decrementer la valeur de la sémaphore**/
                    exit(EXIT_FAILURE);
                }
                update_student(filtre_update, value_update, &db->data[i], &temporary); /**Section critique**/
                if (sem_post(&semaphore)!= 0){                         /** incrementer la valeur de la sémaphore**/
                    exit(EXIT_FAILURE);
                }
            }
        }
        i++;
    }
    clock_t end = clock();
    double execution_time = (double)(end-start)/CLOCKS_PER_SEC * 1000;
    fprintf(file,"Query \"%s\" Completed in %lf ms with %zu results\n",query_write ,execution_time, temporary.lsize);
    write_db_file(&temporary, file);
    if (isatty(0)!=0) printf("%s\n", filename);
    fclose(file);
}

void create_logfile(){
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0777);
    }
}

void * execute_request(void* query){
    char query_to_write[QUERY_SIZE];
    strcpy(query_to_write, query);
    char* save = query;
    char* command;
    char filtre[QUERY_SIZE];
    char value[QUERY_SIZE];
    char fi[QUERY_SIZE];
    char va[QUERY_SIZE];
    struct tm* date= malloc(sizeof(struct tm));
    unsigned *identifier = malloc(sizeof(unsigned));
    command = strtok_r(NULL, " ", &save);
    if (strcmp(command, "insert")==0) {
        if (parse_insert(save, filtre, value, identifier, va, date) == 1) {
            insert_student(filtre, value, identifier, va, date);
        }
    }
    else if (strcmp(command, "delete")==0 || strcmp(command, "select")==0){
        if (parse_selectors(save, filtre, value) == 1){
            select_rq(&data, command, filtre, value, fi, va, query_to_write);
        }
        else {
            printf("Invalid Request, Please verify syntaxe & try again\n");}
    }
    else if(strcmp(command, "update")==0){
        if(parse_update(save, filtre, value, fi, va) == 1){
            select_rq(&data, command, filtre, value, fi, va, query_to_write);
        }
        else {printf("Invalid Request, Please verify syntax & try again\n");}
    }
    else {printf("Invalid Request, Please verify syntax & try again\n");}
    return NULL;
}

void multiple_exec(char* request, char liste[NUMBER_OF_QUERIES][QUERY_SIZE]) {
    char c;
    int index = 0;
    int position = 0;
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            request[index] = c;
            index++;
        } else {
            request[index] = '\0';
            strcpy(liste[position],request);
            position++;
            index=0;
        }
    }
    if (c == EOF) {
        if (strlen(request) > 6) {
            request[index] = '\0';
            strcpy(liste[position],request);
        }
    }
}
void single_exec(){
    bool start_execution = true;
    while (start_execution){
        printf("> ");
        char query[QUERY_SIZE];
        int index = 0;
        char caracter;
        while((caracter=getchar()) != EOF){
            query[index] = caracter;
            index++;
            if (caracter=='\n'){
                query[index-1] = '\0';
                execute_request(query);
                break;
            }
        }
        if (caracter==EOF){
            start_execution = false;
            db_save(&data, binary_file_path);
        }
    }
}

void compute_threads(char liste[NUMBER_OF_QUERIES][QUERY_SIZE], int* number_of_threads){
    *(number_of_threads) = 0;
    while(1){
        if(strlen(liste[*(number_of_threads)])>6){
            (*number_of_threads)++;
        }
        else {
            break;
        }
    }
}