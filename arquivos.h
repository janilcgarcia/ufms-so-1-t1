#ifndef _ARQUIVOS_H_
#define _ARQUIVOS_H_

#include "shell.h"

void criar_arquivo (shell_ctx_t *ctx, char **args);
void adiciona_conteudo (shell_ctx_t *ctx, char **args);
void remove_arquivo (shell_ctx_t *ctx, char **args);

void lock_arquivo (shell_ctx_t *ctx, char **args);
void free_arquivo (shell_ctx_t *ctx, char **args);

void kill_processo (shell_ctx_t *ctx, char **args);
void leak_bytes (shell_ctx_t *ctx, char **args);

void meminfo (shell_ctx_t *ctx, char **args);

#endif /* _ARQUIVOS_H_ */
