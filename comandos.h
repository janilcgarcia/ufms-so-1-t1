#ifndef _COMANDOS_H_
#define _COMANDOS_H_

#include "shell.h"

typedef void (*comando_fn)(shell_ctx_t *ctx, char **args);

typedef struct {
  char nome[20];
  comando_fn comando;
} comando_t;

comando_fn procurar_comando(comando_t *lst, int len, char *nome);

#endif /* _COMANDOS_H_ */
