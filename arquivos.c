#define _SVID_SOURCE

#include "arquivos.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>

#include "args.h"

volatile sig_atomic_t sig_err;
volatile sig_atomic_t sig_sent;

static void
sigusr1_handler (int sig)
{
  sig_err += 1;
}

static void
sigusr2_handler (int sig)
{
  sig_sent = 1;
}

static void
nada (int sig)
{
}

void criar_arquivo(shell_ctx_t *ctx, char **args)
{
  int fd;

  if (strlen (*args) <= 0)
    {
      printf ("Uso: fcreate fileName\n");
      return;
    }

  fd = open (*args, O_CREAT, S_IRUSR|S_IWUSR);
  close (fd);
}

void adiciona_conteudo (shell_ctx_t *ctx, char **args)
{
  int fd;
  char *nome;
  char *conteudo;

  if (args_len (args) < 2)
    {
      printf("Faltam arugmentos. Uso: fadd nome_do_arquivo mensagem\n");
      return;
    }

  nome = *args;
  conteudo = *(args + 1);
  	
  fd = open(nome, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR); 
  if (fd == -1)
    {
      fprintf (stderr, "Não foi possível abrir o arquivo %s: %s\n",
	       nome,
	       strerror(errno));
      return;
    }
  else
    {
      write(fd, conteudo, strlen(conteudo));
      fd = close(fd);
	
      if (fd == -1)
	{
	  printf("Erro ao fechar o arquivo %s: %s\n", nome, strerror(errno));
	  return;
	}
    }
}

void remove_arquivo (shell_ctx_t *ctx, char **args)
{
  int err;
  
  if (args_len (args) < 1)
    {
      fputs ("Faltam arugmentos. ", stderr);
      fputs ("Uso: fremove nome_do_arquivo\n", stderr);
      return;
    }

  err = unlink (*args);

  if (err)
    {
      fprintf (stderr, "Erro ao remover o arquivo: %s\n", strerror (errno));
    }
}

static void
_lock_arquivo(shell_ctx_t *ctx, char *nome, int tipo)
{
  pid_t cid;
  int fd;
  int res, op;

  struct sigaction usr1_action, usr1_old;
  struct sigaction usr2_action, usr2_old;
  struct sigaction ignore;

  ignore.sa_flags = 0;
  sigemptyset (&ignore.sa_mask);
  ignore.sa_handler = nada;

  usr1_action.sa_flags = 0;
  sigemptyset (&usr1_action.sa_mask);
  usr1_action.sa_handler = sigusr1_handler;

  usr2_action.sa_flags = 0;
  sigemptyset (&usr2_action.sa_mask);
  usr2_action.sa_handler = sigusr2_handler;

  sig_err = 0;
  sig_sent = 0;

  sigaction(SIGUSR1, &usr1_action, &usr1_old);
  sigaction(SIGUSR2, &usr2_action, &usr2_old);
  
  cid = fork ();

  if (cid == -1)
    {
      fputs("Falha em abrir o processo que travaria o arquivo!\n", stderr);
      return;
    }
  else if (cid == 0)
    {
      sigaction (SIGQUIT, &ignore, NULL);
      fd = open (nome, O_RDONLY);

      if (fd == -1)
	{
	  kill (getppid (), SIGUSR1);
	  usleep (200);
	  kill (getppid (), SIGUSR2);
	  
	  _exit(1);
	}

      switch (tipo)
	{
	case 1:
	  op = LOCK_EX;
	  break;

	case 2:
	  op = LOCK_SH;
	  break;
	}
      

      res = flock (fd, op | LOCK_NB);

      if (res != 0)
	{
	  kill (getppid (), SIGUSR1);
	  usleep (200);
	  kill (getppid (), SIGUSR1);
	  usleep (200);
	  kill (getppid (), SIGUSR2);

	  close (fd);

	  _exit (1);
 	}

      kill (getppid (), SIGUSR2);
      pause ();

      flock (fd, LOCK_UN);
      close (fd);
      _exit (0);
    }
  else
    {
      while (!sig_sent)
	{
	  pause();
	}

      if (!sig_err)
	{
	  printf("%ld\n", (long) cid);
	  add_lock (ctx, nome, cid);
	}
      else
	{
	  switch (sig_err)
	    {
	    case 1:
	      fprintf(stderr, "Não foi possível abrir o arquivo %s\n", nome);
	      break;

	    case 2:
	      fprintf(stderr,
		      "Uma trava de tipo incompatível "
		      "parece já existir no arquivo %s.\n", nome);
	      break;
	    }
	  
	  waitpid (cid, NULL, 0);
	}
    }

  sigaction(SIGUSR1, &usr1_old, NULL);
  sigaction(SIGUSR2, &usr2_old, NULL);
}

void
lock_arquivo (shell_ctx_t *ctx, char **args)
{
  char *nome, *str_tipo, *res_str;
  int tipo;

  if (args_len (args) < 2)
    {
      fputs("Faltam argumentos para esta operação.\n", stderr);
      fputs("Uso: flock nome_do_arquivo [1|2]\n", stderr);
      return;
    }

  nome = *args;
  str_tipo = *(args + 1);

  tipo = strtol(str_tipo, &res_str, 10);

  if (*str_tipo == '\0' || *res_str != '\0')
    {
      fprintf(stderr, "O argumento \"%s\" não é um número válido!\n",
	      str_tipo);
      return;
    }

  if (tipo < 1 || tipo > 2)
    {
      fprintf(stderr, "Tipo %d é inválido, deve ser 1 - exclusivo ou 2 - compartilhado!\n",
	      tipo);
      return;
    }

  _lock_arquivo(ctx, nome, tipo);
}

static void
free_arquivo_pid (shell_ctx_t *ctx, pid_t pid)
{
  if (!kill (pid, SIGQUIT))
    {
      waitpid (pid, NULL, 0);
    }
}

static void
free_arquivo_nome (shell_ctx_t *ctx, const char *nome)
{
  int i;
  int s;
  struct file_lock *lock = find_by_name (ctx, nome);

  if (!lock)
    return;

  s = lock->pids_len;

  for (i = 0; i < s; i++)
    {
      free_arquivo_pid(ctx, lock->pids[i]);
      remove_lock(ctx, nome, lock->pids[i]);
    }
}

void
free_arquivo (shell_ctx_t *ctx, char **args)
{
  char *str, *strend;
  pid_t pid;

  if (args_len (args) < 1)
    {
      fputs ("Faltam argumentos para esta operação. ", stderr);
      fputs ("Uso: ffree [nome_do_arquivo|pid]\n", stderr);
      return;
    }

  if (strlen (*args) <= 0)
    {
      fputs ("O nome do arquivo não pode ter tamanho nulo.\n", stderr);
      return;
    }

  str = *args;
  pid = (pid_t) strtol (str, &strend, 10);

  if (*strend == '\0')
    {
      free_arquivo_pid (ctx, pid);
    }
  else
    {
      free_arquivo_nome (ctx, *args);
    }
}

void
kill_processo (shell_ctx_t *ctx, char **args) {
  pid_t pid;
  int err;
  char *retstr;

  if (args_len (args) < 1)
    {
      fputs ("Uso: pkill pid\n", stderr);
      return;
    }

  if (strlen (*args) < 1)
    {
      fputs ("pid inválido!\n", stderr);
      return;
    }

  pid = (pid_t) strtol (*args, &retstr, 10);

  if (*retstr != '\0')
    {
      printf("O Pid só pode ter números\n");
      return;
    }
  
  err = kill (pid, SIGKILL);
  
  if (err < 0)
    {
      printf("Não foi possível matar o processo %ld: %s\n", (long) pid, strerror(errno));
    }
}

void
leak_bytes (shell_ctx_t *ctx, char **args)
{
  int nbytes;
  char *bytes;
  char *res;
  pid_t cpid;
  int i;
  int fd;
  char fnome[40];

  if (args_len (args) < 1)
    {
      nbytes = 16;
    }
  else
    {
      nbytes = strtol (*args, &res, 10);

      if (*args == '\0' || *res != '\0')
	{
	  fprintf (stderr, "Argumento inválido '%s'\n", *args);
	  return;
	}
    }

  if (nbytes <= 0)
    {
      fputs ("A quantidade de bytes deve ser maior que 0.\n", stderr);
      return;
    }

  bytes = malloc (nbytes);

  if (!bytes)
    {
      fprintf (stderr, "Não foi possível alocar %d bytes.\n", nbytes);
      return;
    }

  for (i = 0; i < nbytes; i++)
    bytes[i] = 'a';

  cpid = fork ();

  if (cpid == -1)
    {
      fputs ("Impossível criar um processo filho...\n", stderr);
      free(bytes);
    }
  else if (cpid == 0)
    {
      sprintf (fnome, "%ld", (long) getpid ());
      fd = open(fnome, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

      write (fd, bytes, nbytes);
      close (fd);

      free(bytes);
      
      _exit(0);
    }
  else
    {
      waitpid (cpid, NULL, 0);
      free (bytes);
    }
}

typedef struct processo
{
  int pid;
  char comm[400];
  char state;
  int ppid;
  int pgrp;
  int session;
  int tty_nr;
  int tpgid;
  unsigned long flags;
  unsigned long minflt;
  unsigned long cminflt;
  unsigned long maiflt;
  unsigned long cmaiflt;
  unsigned long utime;
  unsigned long stime;
  long cutime;
  long cstime;
  long priority;
  long nice;
  long num_threads;
  long itrealvalue;
  unsigned long long starttime;
  unsigned long vsize;
  long rss;
} processo_t;

static int
isstrnum (const char *str)
{
  int i;
  int r = 1;

  for (i = 0; i < strlen(str); i++)
    if (!isdigit(str[i]))
      r = 0;

  return r;
}

static int
filter_num (const struct dirent *entry)
{
  return isstrnum (entry->d_name);
}

void
meminfo (shell_ctx_t *ctx, char **args)
{
  struct dirent **namelist;
  int n;
  int i;
  FILE *f;
  char path[400];
  processo_t proc;

  n = scandir("/proc", &namelist, filter_num, alphasort);

  if (n < 0)
    {
      fprintf (stderr, "Não foi possível abrir o diretório /proc\n");
      return;
    }

  printf ("PID\tNOME\tESTADO\t\tVSIZE(bytes)\tRSS(bytes)\n");

  for (i = 0; i < n; i++)
    {
      strcpy (path, "/proc/");
      strcat (path, namelist[i]->d_name);
      strcat (path, "/stat");

      f = fopen (path, "r");

      fscanf (f, "%d %s %c %d %d %d %d %d %lu %lu %lu %lu %lu"
	      "%lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld",
	      &proc.pid, proc.comm, &proc.state, &proc.ppid,
	      &proc.pgrp, &proc.session, &proc.tty_nr, &proc.tpgid,
	      &proc.flags, &proc.minflt, &proc.cminflt, &proc.maiflt,
	      &proc.cmaiflt, &proc.utime, &proc.stime, &proc.cutime,
	      &proc.cstime, &proc.priority, &proc.nice, &proc.num_threads,
	      &proc.itrealvalue, &proc.starttime, &proc.vsize, &proc.rss);

      fclose(f);

      const char *estado;

      switch (proc.state)
      	{
      	case 'R':
      	  estado = "Rodando";
      	  break;

      	case 'S':
      	  estado = "Dormindo";
      	  break;

      	case 'D':
	  estado = "Dormindo - Disco";
	  break;

	case 'Z':
	  estado = "Zumbi";
	  break;

	case 'T':
	  estado = "Parado";
	  break;

	case 't':
	  estado = "Parado (tracing)";
	  break;

	case 'X':
	case 'x':
	  estado = "Morto";
	  break;

	case 'K':
	  estado = "Wakekill";
	  break;

	case 'W':
	  estado = "Acordando";
	  break;

	case 'P':
	  estado = "Estagnado";
	  break;
      	}

      printf ("%d\t%s\t%s\t%lu\t%ld\n", proc.pid, proc.comm, estado, proc.vsize, proc.rss);

      free (namelist[i]);
    }

  free (namelist);
}
