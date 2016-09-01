#ifndef _COMANDOS_H_
#define _COMANDOS_H_

typedef void (*comando_fn)(char *args);

typedef struct {
  char nome[20];
  comando_fn comando;
} comando_t;

comando_fn procurar_comando(comando_t *lst, int len, char *nome, int nome_len);

#endif /* _COMANDOS_H_ */
