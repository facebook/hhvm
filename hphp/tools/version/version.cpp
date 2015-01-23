#include <stdio.h>
#include "hphp/runtime/version.h"

/**
 * Quickly dump HHVM_VERSION. Intended usage is by the packaging script to
 * quickly verify that it's building the thing that it thinks it's building.
 */
int main(void) {
  printf("%s\n", HHVM_VERSION);
  return 0;
}
