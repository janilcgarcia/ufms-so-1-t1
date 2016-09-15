#include "shell.h"
#include <stdlib.h>
#include <string.h>


void file_lock_init(struct file_lock *lock)
{
  lock->pids_len = 0;
}

void shell_ctx_init(struct shell_ctx *ctx)
{
  ctx->locks_len = 0;
}


void
add_lock (struct shell_ctx *ctx, const char *filename, pid_t pid)
{
  struct file_lock *lock;
  int i;

  lock = NULL;

  for (i = 0; i < ctx->locks_len && !lock; i++)
    {
      if (strcmp (ctx->locks[i].filename, filename) == 0)
	lock = ctx->locks + i;
    }

  if (lock == NULL)
    { 
      lock = ctx->locks + (ctx->locks_len++);

      file_lock_init(lock);
      
      strcpy(lock->filename, filename);
    }

  for (i = 0; i < lock->pids_len; i++)
    {
      if (pid == lock->pids[i])
	return;
    }

  lock->pids[lock->pids_len++] = pid;
}

void
remove_lock (struct shell_ctx *ctx, const char *filename,
	     pid_t pid)
{
  struct file_lock *lock;
  int i;
  int li;

  lock = NULL;

  for (i = 0; i < ctx->locks_len && !lock; i++)
    {
      if (strcmp (ctx->locks[i].filename, filename) == 0)
	{
	  lock = ctx->locks + i;
	  li = i;
	}
    }

  if (!lock)
    return;

  for (i = 0; i < lock->pids_len; i++)
    {
      if (lock->pids[i] == pid)
	break;
    }

  if (i == lock->pids_len)
    return;

  for (i = i + 1; i < lock->pids_len; i++)
    lock->pids[i - 1] = lock->pids[i];

  if (--(lock->pids_len) <= 0)
    {
      for (i = li + 1; i < ctx->locks_len; i++)
	ctx->locks[i - 1] = ctx->locks[i];
    }
}

struct file_lock *
find_by_name (struct shell_ctx *ctx, const char *name)
{
  int i;
  struct file_lock *lock;

  lock = NULL;

  for (i = 0; i < ctx->locks_len && !lock; i++)
    if (strcmp (ctx->locks[i].filename, name) == 0)
      lock = ctx->locks + i;

  return lock;
}
