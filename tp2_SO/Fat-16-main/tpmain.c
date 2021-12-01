#include "tp.h"

int main()
{

  int carregado = 0; // variavel 0-fat não carregada e 1-para fat carregada
  //strings que recebe a entrada e a separa
  char *linhaComando = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  char *comando = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  char *parametros = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  do
  {
    //limpa as strings
    strcpy(linhaComando, "");
    strcpy(comando, "");
    strcpy(parametros, "");
    printf("$");
    gets(linhaComando);
    fflush(stdin);
    //separa a entrada em comandao e seus parametros em uma string
    separaString(linhaComando, comando, parametros, " ");
    if (carregado == 1 || strcmp(comando, "load") == 0 || strcmp(comando, "init") == 0 || strcmp(comando, "exit") == 0)
    { //verifica se a fat foi carregada
      if (strcmp(comando, "init") == 0 && strcmp(parametros, "") == 0)
      { //chama a função init
        carregado = init();
      }
      else if (strcmp(comando, "load") == 0 && strcmp(parametros, "") == 0)
      { //chama a função load
        carregado = load();
      }
      else if (strcmp(comando, "ls") == 0)
      { //chama a função ls
        ls(parametros);
      }
      else if (strcmp(comando, "mkdir") == 0)
      { //chama a função mkdir
        mkdir(parametros);
      }
      else if (strcmp(comando, "create") == 0)
      { //chama a função create
        create(parametros);
      }
      else if (strcmp(comando, "unlink") == 0)
      { //chama a função unlink
        unlink(parametros);
      }
      else if (strcmp(comando, "write") == 0)
      { //chama a função write
        write(parametros);
      }
      else if (strcmp(comando, "append") == 0) // mkdir pasta criar
      {                                        //chama a função append
        append(parametros);
      }
      else if (strcmp(comando, "read") == 0)
      { //chama a função read
        read(parametros);
      }
      else if (strcmp(comando, "exit") == 0)
      { //desaloca as strings e fecha o programa
        free(linhaComando);
        free(comando);
        free(parametros);
        return 0;
      }
      else
      {
        printf("COMANDO INVALIDO\n");
      }
    }
    else
    {
      printf("ERRO NA FAT, UTILIZE INIT OU LOAD\n");
    }
  } while (1);
}