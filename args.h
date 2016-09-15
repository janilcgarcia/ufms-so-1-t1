#ifndef _ARGS_H_
#define _ARGS_H_

#define ERR_OK 0
#define ERR_QUOTE 1

/**
 * Lê uma lista de tokens da entrada padrão. Todo que estiver entre
 * aspas simples (') or duplas(") é tratado como um só elemento.
 * @param err Caso aconteça algum erro grave na função ela retornará
 * NULL e o erro será armazenado em err.
 * @return Uma lista alocada em heap de argumentos terminada com um
 * elemento com o valor NULL. Deve-se desalocar a lista usando free_args.
 */
char **get_args(int *err);

/**
 * Desaloca uma lista de argumento alocada com get_args,
 * @param args Lista de argumentos a ser desalocada.
 */
void free_args(char **args);

int args_len(char **args);

#endif /* _ARGS_H_ */
