#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/**
	Copia nBytes del archivo origen al archivo destino.
	origin: puntero al comienzo del archivo que vamos a copiar.
	destination: puntero al comienzo del archivo que vamos a pegar.
	nBytes: numero de bytes que queremos copiar de origen a destino.

	return: 0 en caso de error, tamaño de datos copiados en caso de todo OK.
**/
int copynFile(FILE * origin, FILE * destination, int nBytes)
{

	int nCopy = 0;
	int c;
	while (nCopy < nBytes && (c = getc(origin)) != EOF) {
		putc((unsigned char) c, destination);
		nCopy++;
	}
	return (nCopy);
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char* loadstr(FILE * file)
{
	int n, size = 0;
	char* buf;

	do {
		n = getc(file);
		size++;
	} while ((n != (int) '\0')
		 && (n != EOF));

	if (n == EOF)
		return NULL;

	if ((buf = (char *) malloc(size)) == NULL)
		return NULL;

	fseek(file, -size, SEEK_CUR);

	fread(buf, 1, size, file);

	return buf;
}

/**
	Lee la cabecera de un tarbal y la aloja en memoria.
	
	tarFile: puntero al comienzo del archivo tarball.
	nFile: parametro de salida. Usado para devolver el numero de archivos que tiene el tarball.
	
	En caso de éxito, devolveremos además el array de tipo stHeaderEntry alojado en memoria dinámica.
**/
stHeaderEntry* readHeader(FILE * tarFile, int *nFiles)
{
	int i,j;
	stHeaderEntry *header;

	// #files
	fread(nFiles, sizeof (int), 1, tarFile);

	// head of tar
	if ((header = (stHeaderEntry *) malloc(sizeof (stHeaderEntry) * (*nFiles))) == NULL) {
		perror("Error in memory allocation for the header's file mtar");
		fclose(tarFile);
		return NULL;
	}

	//Load header's file
	for (i = 0; i < *nFiles; i++) {
		if ((header[i].name=loadstr(tarFile))==NULL) {
			for (j = 0; j < *nFiles; j++)
				free(header[j].name);
			free(header);
			fclose(tarFile);
			return NULL;
		}
		fread(&header[i].size, sizeof(header[i].size), 1, tarFile);
	}

	return header;
}

/**
	Crea un archivo "tarball"
	nFiles: Numero de archivos que vamos a guardar en el tarball
	filenames: Array con el nombre de los archivos que vamos a incluir en el tarball
	tarname: nombre del archivo tarball
	
	En caso de exito, devolvemos "EXIT_SUCCESS". EN caso de error, devolvemos "EXIT_FAILURE".
	(Macros definidas en stdlib.h)

**/
int createTar(int nFiles, char *fileNames[], char tarName[]){
	int i,j;
	FILE *tarFile, *inputFile;
	stHeaderEntry *header;
	unsigned int headersize;

	//file[s]
	if (nFiles <= 0) {
		fprintf(stderr,"%s",use);
		return (EXIT_FAILURE);
	}
	//Open destination file
	if ((tarFile = fopen(tarName, "wx")) == NULL) {
		fprintf(stderr, "The mtar file %s could not be opened: ", tarName);
		perror(NULL);
		return (EXIT_FAILURE);
	}
	//Memory reservation for the header structure of mtar file[s]
	if ((header = malloc(sizeof (stHeaderEntry) * nFiles)) == NULL) {
		perror("Error at memory allocation in mtar file header");
		fclose(tarFile);
		remove(tarName);
		return (EXIT_FAILURE);
	}
	//Compute #bytes needed for the header and
	//fill the header with the file name[s]
	headersize = sizeof (int);
	for (i = 0; i < nFiles; i++) {
		int namesize = strlen(fileNames[i]) + 1;

		header[i].name = (char *) malloc(namesize);
		if (header[i].name == NULL) {
			perror("Error at memory allocation for the file name");
			fclose(tarFile);
			remove(tarName);
			for (j = 0; j < i; j++)
				free(header[j].name);
			free(header);
			return (EXIT_FAILURE);
		}
		strcpy(header[i].name, fileNames[i]);

		headersize += namesize + sizeof (header->size);
	}
	//Seek in mtar data area
	fseek(tarFile, headersize, SEEK_SET);

	//Fill header info (at RAM memory) and copy file[s] data into tar
	for (i = 0; i < nFiles; i++) {
		//Source file[s] open
		if ((inputFile = fopen(fileNames[i], "r")) == NULL) {
			fprintf(stderr, "Not possible to open input file %s: \n", fileNames[i]);
			perror(NULL);
			fclose(tarFile);
			remove(tarName);
			for (j = 0; j < nFiles; j++)
				free(header[j].name);
			free(header);
			return (EXIT_FAILURE);
		}
		//File[s] copy
		header[i].size = copynFile(inputFile, tarFile, INT_MAX);
		fclose(inputFile);
	}

	//Write each header file[s] with the file[s]
	rewind(tarFile);
	fwrite(&nFiles, sizeof (int), 1, tarFile);
	for (i = 0; i < nFiles; i++) {
		fwrite(header[i].name, 1, strlen(header[i].name) + 1, tarFile);
		fwrite(&header[i].size, sizeof (header[i].size), 1, tarFile);
	}

	fprintf(stdout, "mtar file created successfully\n");

	for (j = 0; j < nFiles; j++)
		free(header[j].name);
	free(header);
	fclose(tarFile);

	return (EXIT_SUCCESS);
}

/**
	Extrae archivos guardados en un archivo tarball.
	tarName: nombre del archivo tarball.
	
	En caso de exito, devuelve "EXIT_SUCCESS". En caso de error, devolvemos "EXIT_FAILURE"

**/
int extractTar(char tarName[]){
	
    stHeaderEntry *cabecera = NULL;
    int numeroArchivos, i = 0, copiar = 0;
    //int tamanioNombre = 0;
    FILE *tarFile = NULL;
 
    /*Abrir archivo*/
    if((tarFile = fopen(tarName, "r")) == NULL){
        printf("Error al abrir el archivo %s", tarName);
        return EXIT_FAILURE;
    }
 
    //Lectura de cabecera
    cabecera = readHeader(tarFile, &numeroArchivos);
    for(i = 0; i < numeroArchivos; i++){
        //Creacion y escritura sobre los ficheros
        FILE *archNuevo = fopen(cabecera[i].name, "w");
        if(archNuevo == NULL){
            printf("El fichero %s no ha podido abrirse", cabecera[i].name);
            for(i = 0; i < numeroArchivos; i++)
		//Liberamos memoria...
                free(cabecera[i].name);
            free(cabecera);
            fclose(tarFile);
            return (EXIT_FAILURE);
        }
        copiar = copynFile(tarFile, archNuevo, cabecera[i].size);
        if(copiar == -1){ 
            printf("El fichero no ha podido copiarse");
            for(i = 0; i < numeroArchivos; i++)
                free(cabecera[i].name);
            free(cabecera);
            fclose(tarFile);
            return (EXIT_FAILURE);
        }
        fclose(archNuevo);
    }
    
    for (i = 0; i < numeroArchivos; i++)
        free(cabecera[i].name);
    free(cabecera);
    fclose(tarFile);
    // Complete the function
    return (EXIT_SUCCESS);
         
	return(EXIT_SUCCESS);
}

