#ifndef _ARQUIVOS_H_
#define _ARQUIVOS_H_

void criar_arquivo(char *args);
void adiciona_conteudo(char *args);
void remove_arquivo(char *args);

void lock_arquivo(char *args);
void free_arquivo(char *args);

void kill_processo(char *args);
void leak_bytes(char *args);

void meminfo(char *args);

#endif /* _ARQUIVOS_H_ */
