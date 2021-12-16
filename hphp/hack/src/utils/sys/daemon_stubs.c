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
  /* Allow users to opt out of this behavior in restricted environments, e.g.
     docker with default seccomp profile */
  if (getenv("HHVM_DISABLE_PERSONALITY")) {
    CAMLreturn(Val_unit);
  }

  int res = personality((unsigned long)0xffffffff);
  if (res == -1) {
      fprintf(stderr, "error: daemon_stubs.c: caml_disable_ASLR: failed to get personality\n");
      exit(1);
  }
  if (! (res & ADDR_NO_RANDOMIZE)) {
    res = personality((unsigned long)(res | ADDR_NO_RANDOMIZE));
    if(res == -1) {
      fprintf(stderr, "error: daemon_stubs.c: caml_disable_ASLR: failed to set personality\n");
      exit(1);
    }
    int i, argc = Wosize_val(args);
    char const** argv = (char const**)(malloc ((argc + 1) * sizeof(char const*)));
    for (i = 0; i < argc; ++i) {
      argv[i] = String_val(Field(args, i));
    }
    argv[argc] = (char const*)0;
    (void)execv(argv[0], (char *const *)argv); /* Usually no return. */
  }
#endif
  /* Reachable on MacOS, or if the execv fails. */
  CAMLreturn(Val_unit);
}
