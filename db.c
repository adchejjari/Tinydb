/*
 * Auteur : Adnane CHEJJARI
 * Matricule : 492759
 * Date : Decembre 2020
 * PROJET 2 - INFO-F201 : Systèmes d'exploitation
 * */

#include "db.h"

void db_add(database_t *db, student_t s){
    if (db->lsize >= db->psize){
        db->psize *= 2;
        db->data = realloc(db->data,sizeof(student_t) * (db->psize));
    }
    memcpy(&(db->data[db->lsize]), &s, sizeof(student_t));
    db->lsize++;
}

void db_delete(database_t *db, student_t *s){
    int position = 0;
    while(position<=db->lsize){
        if (student_equals(&(db->data[position]), s) == 1){break;}
        else{ position++;}
    }
    int i;
    for(i = position; i<db->lsize; i++){db->data[i] = db->data[i+1];}
    db->lsize--;
}


void db_save(database_t *db, const char *path) {
    FILE *filep;
    filep = fopen(path, "wb");
    if (filep==NULL){
        printf("fopen failed\n");
        exit(EXIT_FAILURE);}
    int i;
    for(i = 0; i<db->lsize; i++){
        fwrite(&db->data[i], sizeof(student_t), 1, filep);
    }
    printf("Comitting database changes to the disk...\nDone\n");
    fclose(filep);
}

void db_load(database_t *db, const char *path){
    printf("Loading the database...\n");
    FILE * filep;
    filep = fopen(path, "rb");
    if (filep==NULL){
        printf("fopen failed\n");
        exit(EXIT_FAILURE);
    }
    student_t student;
    while(fread(&student, sizeof(student_t),1, filep)){
        db_add(db, student);
    }
    fclose(filep);
    printf("Done!\n");
}

void db_init(database_t *db){
    db->lsize = 0;
    db->psize = 1;
    db->data = (student_t*)malloc(sizeof(student_t) * db->psize);
}