#include "comandos.h"
#include <string.h>

comando_fn procurar_comando(comando_t *lst, int len, char *nome, int nome_len)
{
  int i;
  
  for (i = 0; i < len; i++) {
    if (strncmp(lst[i].nome, nome, nome_len) == 0)
      return lst[i].comando;
  }

  return NULL;
}
