#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include "shell.h"
#include "comandos.h"
#include "arquivos.h"
#include "args.h"

#define LIST_LEN 8

enum estado_entrada {
  EE_OK,
  EE_SAIR,
  EE_ERRO
};

static enum estado_entrada
processa_args(shell_ctx_t *ctx, comando_t *list, int len,
	      char **args, int err)
{
  comando_fn fn;
  
  if (args == NULL)
    {
      switch (err)
	{
	case ERR_QUOTE:
	  fputs("Faltando aspas!\n", stderr);
	  break;
	  
	default:
	  fputs("Existe algum erro na sua entrada!\n", stderr);
	  break;
	}
      
      return EE_SAIR;
    }

  if (strcmp(args[0], "sair") == 0)
    return EE_SAIR;

  fn = procurar_comando(list, len, args[0]);

  if (!fn)
    {
      fprintf(stderr, "Comando não encontrado: %s\n", args[0]);
      return EE_ERRO;
    }

  fn(ctx, args + 1);

  return EE_OK;
}

int
main (int argc, char **argv)
{
  comando_t list[LIST_LEN];

  shell_ctx_t ctx;

  char **args = NULL;
  int err;
  int rodando = 1;
  enum estado_entrada ee;

  struct sigaction sa;

  shell_ctx_init(&ctx);

  strcpy(list[0].nome, "fcreate");
  list[0].comando = criar_arquivo;

  strcpy(list[1].nome, "fadd");
  list[1].comando = adiciona_conteudo;

  strcpy(list[2].nome, "flock");
  list[2].comando = lock_arquivo;

  strcpy(list[3].nome, "fremove");
  list[3].comando = remove_arquivo;

  strcpy(list[4].nome, "ffree");
  list[4].comando = free_arquivo;

  strcpy(list[5].nome, "pkill");
  list[5].comando = kill_processo;

  strcpy(list[6].nome, "mleaks");
  list[6].comando = leak_bytes;

  strcpy(list[7].nome, "mmeminfo");
  list[7].comando = meminfo;

  while (rodando)
    {
      fputs("-> ", stdout);
      args = get_args(&err);

      ee = processa_args(&ctx, list, LIST_LEN, args, err);

      switch (ee)
	{
	case EE_SAIR:
	  rodando = 0;
	  break;

	default:
	  break;
	}

      free_args(args);
    }

  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sigaction (SIGQUIT, &sa, NULL);

  kill (-getpid (), SIGQUIT);

  while (wait(NULL) != -1);

  return 0;
}
