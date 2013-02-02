#ifndef _INTERNAL_H
#define _INTERNAL_H

#include <stddef.h>

static inline void set_error(
    struct afdt_error_t* err,
    enum afdt_operation operation,
    const char* message) {
  if (err != NULL) {
    err->operation = operation;
    err->message = message;
  }
}

#endif // _INTERNAL_H
