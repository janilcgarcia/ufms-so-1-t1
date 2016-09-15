#ifndef _SHELL_H_
#define _SHELL_H_

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

struct file_lock {
  char filename[250];
  pid_t pids[150];
  size_t pids_len;
};

typedef struct shell_ctx {
  struct file_lock locks[40];
  size_t locks_len;
} shell_ctx_t;


void file_lock_init (struct file_lock *lock);
void shell_ctx_init (struct shell_ctx *ctx);

void file_lock_free (struct file_lock *lock);
void shell_ctx_free (struct shell_ctx *ctx);

void add_lock (struct shell_ctx *ctx, const char *filename,
	       pid_t pid);
void remove_lock (struct shell_ctx *ctx, const char *filename,
		  pid_t pid);

struct file_lock *find_by_name (struct shell_ctx *ctx, const char *name);

#endif /* _SHELL_H_ */
