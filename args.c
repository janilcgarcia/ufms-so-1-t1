#include "args.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define SEP " \t"
#define QUOTES "\"'"

/**
 * Modo de leitura atual da linha de comando.
 */
enum mode {
  READING_ARG,
  READING_SPACE,
  READING_QUOTED,
  END
};

/**
 * Concatena o buffer para uma posição permanente de memória.
 * @param output Saíde onde o buffer será armazenado. Esta função
 * cuida da alocação de espaço, então é necessário de passar um
 * ponteiro para ponteiro.
 * @param outputlen O tamanho atual do espaço permanente de memória.
 * O buffer será concatenado a partir desta posição.
 * @param buffer O buffer a ser concatenado.
 * @param bufferlen O tamanho do buffer a ser concatenado.
 */
static inline void
copy_buffer (char **output, size_t *outputlen,
	     char *buffer, size_t *bufferlen)
{
  // Aloca espaço para a nova string. A nova string tem o tamanho
  // de caracteres +1 para que possa armazenar o indicar de
  // terminação '\0'
  *output = realloc (*output, *outputlen + *bufferlen + 1);
  
  strncpy (*output + *outputlen, buffer, *bufferlen);
  *outputlen += *bufferlen;
  *bufferlen = 0;
  
  (*output)[*outputlen] = '\0';
}

/**
 * Lê caracteres até que um caracter separador seja atingido,
 * ou o final da saída padrão.
 * @param output Posição onde a string contendo o conteúdo lido
 * vai ser armazenada.
 * @param buffer Buffer pré-alocado para esta função.
 * @param bufferlen O tamanho máximo do buffer.
 * @param sep Uma string com possíveis separadores. A função para
 * quando qualquer destes for atingido.
 * @return Retorna o último caracter lido.
 */
static int
read_until_sep (char **output,
		char *buffer,
		const size_t bufferlen,
		const char *sep)
{
  int c;
  size_t output_len;
  size_t buffer_pos;

  output_len = buffer_pos = 0;

  c = fgetc (stdin);

  // Lê novo caracter enquando o caracter não for
  // um separador ou fim de arquivo.
  while (!strchr (sep, (char) c) && c != EOF)
    {
      buffer[buffer_pos++] = (char) c;

      // Copia o buffer para a posição permanente se não
      // houver mais espaço no buffer.
      if (buffer_pos >= bufferlen)
	copy_buffer (output,
		     &output_len,
		     buffer,
		     &buffer_pos);

      c = fgetc (stdin);
    }

  // Copia o resto de informações do buffer para a posição
  // permanente.
  copy_buffer (output,
	       &output_len,
	       buffer,
	       &buffer_pos);

  return c;
}

char **
get_args (int *err)
{
  char **output; // Saída da função
  char buffer[BUFFER_SIZE]; // Buffer para a leitura da entrada
  enum mode m = READING_SPACE; // Modo de leitura atual
  int c;
  size_t currentpos; // Posição atual (na lista de argumentos)
  
  char quote[2]; // Variável utilizada para ler conteúdos dentre
                   // aspas
  quote[1] = '\0';

  // Aloca um lista de tamanho "1". Ela só contém a terminação NULL.
  output = malloc (sizeof (char *));
  output[0] = NULL;

  currentpos = -1; // Começa fora da lista, entra na lista no primeiro
  // argumento.

  *err = ERR_OK; // Reseta o valor de erro.
  // Enquanto não terminar de ler a linha de comando, lê mais um
  // caracter e trata ele.
  while (m != END)
    {
      c = fgetc (stdin);

      if (c == EOF)
	m = END;
      
      switch (m) {
      case READING_ARG:
	// Volta c para a stream stdin para que read_until_sep
	// possa ler ele de volta
	ungetc (c, stdin);

        c = read_until_sep (output + currentpos,
			    buffer,
			    BUFFER_SIZE,
			    SEP "\n");
	
	if (c == EOF || c == '\n')
	  m = END;
	else
	  m = READING_SPACE;
	
	break;

      case READING_QUOTED:
	ungetc (c, stdin);
	
	c = read_until_sep (output + currentpos,
			    buffer,
			    BUFFER_SIZE,
			    quote);

	// Se alcançar o fim do arquivo antes de ler um outra
	// aspa, houve um erro na entrada.
	if (!strchr (QUOTES, c))
	  {
	    free_args(output);
	    *err = ERR_QUOTE;
	    return NULL;
	  }
	else
	  m = READING_SPACE;
	
	break;

      case READING_SPACE:
	if (!strchr (SEP "\n", c))
	  {
	    // Se está em modo de leitura de espaço, e c
	    // não é um espaço, aloca espaço para mais um argumento.
	    output = realloc (output, (++currentpos + 2) * sizeof (char *));
	    output[currentpos] = NULL;
	    output[currentpos + 1] = NULL;

	    // Se o argumento é uma aspa (['] ou ["]), então trata
	    // como um conteúdo entre aspas.
	    // Caso contrário devolve o caracter lido para a stream,
	    // e trata como um argumento normal.
	    if (!strchr (QUOTES, c))
	      {
		ungetc (c, stdin);
		m = READING_ARG;
	      }
	    else
	      {
		m = READING_QUOTED;
		quote[0] = (char) c;
	      }
	  }
	else if (((char) c) == '\n')
	  {
	    m = END;
	  }
	break;

      default:
	break;
      }
    }
  
  return output;
}

void
free_args (char **args)
{
  char **original = args;
  
  while (args && *args)
    free(*(args++));

  free(original);
}

int
args_len (char **args)
{
  int l = 0;

  while (*(args++) != NULL)
    l++;

  return l;
}
