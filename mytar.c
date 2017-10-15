/*Defino librerias*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
       
#include "mytar.h"
       
/*Explica el uso de mytar desde consola para la "ayuda"*/
char use[]="Usage: tar -c|x -f file_mytar [file1 file2 ...]\n";

/*Comienza el main*/
int main(int argc, char *argv[]) {

	//Variable opt=opcion
  int opt, nExtra, retCode=EXIT_SUCCESS;
	//Enumerado flag (crear, error, extraer...)
  flags flag=NONE;
	//Puntero a 
	//Nombre del archivo tar
  char *tarName=NULL;
  
  //Minimum args required=3: mytar -tf file.tar
  if(argc < 2){
    fprintf(stderr,"%s",use);
    exit(EXIT_FAILURE);
  }
  //Parse command-line options
  while((opt = getopt(argc, argv, "cxf:")) != -1) {
    switch(opt) {
      case 'c':
        flag=(flag==NONE)?CREATE:ERROR;
        break;
      case 'x':
        flag=(flag==NONE)?EXTRACT:ERROR;
        break;
      case 'f':
        tarName = optarg;
        break;
      default:
        flag=ERROR;
    }
    //Was an invalid option detected?
    if(flag==ERROR){
      fprintf(stderr,"%s",use);
      exit(EXIT_FAILURE);
    }
  }
  
  //Valid flag + arg + file[s]
  if(flag==NONE || tarName==NULL) {
    fprintf(stderr,"%s",use);
    exit(EXIT_FAILURE);
  }
  
  //Argumentos extra
  nExtra=argc-optind;
  
  //Execute the required action
  switch(flag) {
    case CREATE:
      retCode=createTar(nExtra, &argv[optind], tarName);
      break;
    case EXTRACT:
      if(nExtra!=0){
        fprintf(stderr,"%s",use);
        exit(EXIT_FAILURE);
      }
      retCode=extractTar(tarName);
      break;
    default:
      retCode=EXIT_FAILURE;
  }
  exit(retCode);
}

