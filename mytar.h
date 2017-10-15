#ifndef _MYTAR_H
#define _MYTAR_H

#include<limits.h>

typedef enum{
	NONE,
	ERROR,
	CREATE,
	EXTRACT
} flags;

//Estructura de cada entrada de nuestro array de la cabecera del tarball.
typedef struct{
	char *name; 
	unsigned int size;
}stHeaderEntry;


//Crea nuestro archivo tarball uniendo archivos individuales en uno. Función explicada en Mytar_rotines.c
int createTar(int nFiles, char *fileNames[], char tarName[]);
//Extrae nuestro archivo tarball separando cada archivo individual. Función explicada en Mytar_rotines.c
int extractTar(char tarName[]);


#endif /*_MYTAR_H*/
