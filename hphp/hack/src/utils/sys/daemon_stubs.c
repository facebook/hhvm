/* Copyright (c) 2021, Meta Inc. All rights reserved. */

#include <caml/mlvalues.h>
#include <caml/memory.h>

#if defined(__linux__)
#  include <sys/personality.h>
#  include <unistd.h>
#endif

/* Programs using the Daemon module tend to rely heavily on the
   ability to pass closures to the instances of themselves that they
   spawn. Unfortunately, this technique is unstable in the presence of
   ASLR (address space layout randomization) (when unmarshaling
   closures doesn't work e.g. for this reason or another, you'll see
   it manifest as an exception along the lines of
   ```
     (Failure
       "Can't find daemon parameters: (Failure \"input_value: unknown code module <DIGEST>\")")
   ```
   where <DIGEST> is a SHA-1 hash value).

   This function, if necessary, replaces the current process in which
   ASLR is enabled with a new instance of itself in which ASLR is
   disabled. */

CAMLprim value caml_disable_ASLR(value args) {
  CAMLparam1(args);

#if defined(__linux__)
  int p = personality((unsigned long)0xffffffff);
  if (! (p & ADDR_NO_RANDOMIZE)) {
    if(personality((unsigned long)(p | ADDR_NO_RANDOMIZE)) == -1) {
      fprintf(stderr, "error: daemon_stubs.c: caml_disable_ASLR: failed to set personality\n");
      exit(1);
    }
    int i, argc = Wosize_val(args);
    char const* argv[256]; /* Technically, should be ARG_MAX + 1. */
    if (argc > 255) {
      fprintf(stderr, "error: daemon_stubs.c: caml_disable_ASLR: argument list too long\n");
      exit(1);
    }
    for (i = 0; i < argc; ++i) {
      argv[i] = String_val(Field(args, i));
    }
    argv[argc] = (char const*)0;
    (void)execv(argv[0], (char *const *)argv); /* No return. */
  }
#endif

  CAMLreturn(Val_unit);
}
