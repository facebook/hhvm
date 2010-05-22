/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>

using namespace std;

static void report_dlerror() {
  const char *error;
  if (error = dlerror()) {
    cerr << dlerror() << endl;
  }
}

int main(int argc, char **argv) {
  void *handle = dlopen(argv[argc - 1], RTLD_LAZY);
  report_dlerror();

  int(*php_main)(int, char **) = (int(*)(int, char **))dlsym(handle, "main");
  report_dlerror();

  argv[argc - 1] = NULL;
  int ret = php_main(argc - 1, argv);

  dlclose(handle);
  report_dlerror();

  return ret;
}
