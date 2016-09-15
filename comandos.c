#include "comandos.h"
#include <string.h>

comando_fn procurar_comando(comando_t *lst, int len, char *nome)
{
  int i;
  
  for (i = 0; i < len; i++) {
    if (strcmp(lst[i].nome, nome) == 0)
      return lst[i].comando;
  }

  return NULL;
}
