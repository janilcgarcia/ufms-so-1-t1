#include <stdio.h>
#include <string.h>
#include "comandos.h"
#include "arquivos.h"
#include "args.h"
/*
#define LIST_LEN 2

static void hello_world(char *args)
{
  printf("Hello World!\n");
}

int main()
{
  comando_t list[LIST_LEN];
  comando_fn fn;
  char line[200];
  char *s;
  char *c;
  /*int tcomando;*//*

  strcpy(list[0].nome, "fcreate");
  list[0].comando = criar_arquivo;

  strcpy(list[1].nome, "fadd");
  list[1].comando = adiciona_conteudo;

  do {
    printf("> ");
    scanf("%[^\n]", line);
    fgetc(stdin);
    
    c = strpbrk(line, " \n");
    if (c)
      c[0] = '\0';
    else
      c = line + strlen(line) - 1;

    if (strcmp(line, "sair") == 0)
      return 0;
  
    fn = procurar_comando(list, LIST_LEN, line, strlen(line));

    if (fn == NULL) {
      printf("Função desconhecida!\n");
      continue;
    }

    fn(c + 1);
  } while (1);
  
  return 0;
}

*/

int
main (int argc, char **argv)
{
  char **args = NULL;
  char **ss;
  int err;
  
  do {
    if (args)
      free_args(args);
    
    args = get_args(&err);
    ss = args;

    while (ss != NULL && *ss != NULL)
      printf("Argument: %s\n", *(ss++));
  } while (args != NULL && args[0] != NULL && strcmp(args[0], "sair"));

  if (args)
    free_args(args);
  
  return 0;
}
