#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "arquivos.h"

void criar_arquivo(char *args)
{
  int fd;
  char *arg;

  arg = args;
  args = strpbrk(args, " \n");

  if (args)
    args[0] = '\0';

  if (strlen(arg) <= 0) {
    printf("Uso: fcreate fileName\n");
    return;
  }

  fd = open(arg, O_CREAT, S_IRUSR|S_IWUSR);
  close(fd);
}

void adiciona_conteudo(char *args)
{
  int fd;
  char *nome;
  char *conteudo;

  nome = args;
  args = strpbrk(nome, " \t");
}
