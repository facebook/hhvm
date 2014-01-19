/*
   Demo program for libafdt.

   This program is a synchronous version of "catter".  It uses libafdt's
   synchronous client interface to retrieve the remote file descriptor, then
   uses normal stdio functions to do the copying.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <afdt.h>


int main() {
  const char* transfer_fname = "./fd_transfer_sock";

  int got_fd;
  uint32_t res_len = 0;
  struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 };
  struct afdt_error_t err = AFDT_ERROR_T_INIT;
  int ret = afdt_sync_client(
      transfer_fname,
      (const uint8_t*)"",
      0,
      NULL,
      &res_len,
      &got_fd,
      &timeout,
      &err);
  if (ret < 0) {
    fprintf(stderr, "AFDT ERROR: phase=%s operation=%s message=\"%s\" errno=\"%s\"\n",
        afdt_phase_str(err.phase), afdt_operation_str(err.operation), err.message, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (got_fd < 0) {
    fprintf(stderr, "Didn't get a file descriptor.\n");
    exit(EXIT_FAILURE);
  }

  FILE* output = fdopen(got_fd, "w");
  assert(output != NULL);

  char buf[4096];
  while (!feof(stdin)) {
    if (ferror(stdin)) {
      exit(EXIT_FAILURE);
    }
    size_t got = fread(buf, sizeof(buf[0]), sizeof(buf)/sizeof(buf[0]), stdin);
    (void)fwrite(buf, 1, got, output);
    if (ferror(output)) {
      exit(EXIT_FAILURE);
    }
  }

  return 0;
}
