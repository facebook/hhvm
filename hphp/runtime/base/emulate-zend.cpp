/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/emulate-zend.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

namespace HPHP {

int execute_program(int argc, char **argv);

bool check_option(const char *option) {
  // Parameters that can be directly passed through to hhvm.
  static const char *passthru[] = {
  };

  for (int i = 0; i < sizeof(passthru) / sizeof(const char *); i++) {
    if (strcmp(option, passthru[i]) == 0) return true;
  }
  return false;
}

int emulate_zend(int argc, char** argv){
  vector<string> newargv;

  newargv.push_back(argv[0]);
#ifdef PHP_DEFAULT_HDF
  newargv.push_back("-c");
  newargv.push_back(PHP_DEFAULT_HDF);
#endif

  bool lint = false;
  bool show = false;
  bool need_file = true;
  int ini_fd = -1;
  char ini_path[] = "/tmp/php-ini-XXXXXX";
  const char* program = nullptr;

  int cnt = 1;
  while (cnt < argc) {
    if (check_option(argv[cnt])) {
      newargv.push_back(argv[cnt++]);
      continue;
    }
    if (strcmp(argv[cnt], "-l") == 0 || strcmp(argv[cnt], "--lint") == 0) {
      cnt++;
      lint = true;
      continue;
    }
    if (strcmp(argv[cnt], "-r") == 0) {
      if (cnt + 1 == argc) {
        // Hmm, no program fragment passed along. Let hhvm print its usage
        // message?
        newargv.push_back(argv[cnt++]);
        continue;
      }
      assert(cnt + 1 < argc);
      program = argv[cnt + 1];
      need_file = true;
      cnt += 2;
      continue;
    }
    if (strcmp(argv[cnt], "-w") == 0) {
      cnt++;
      show = true;
      continue;
    }
    if (strcmp(argv[cnt], "-v") == 0 || strcmp(argv[cnt], "--version") == 0) {
      newargv.push_back("--version");
      cnt = argc; // no need to check the rest of options and arguments
      need_file = false;
      break;
    }
    if (strcmp(argv[cnt], "-f") == 0 || strcmp(argv[cnt], "--file") == 0) {
      cnt++;
      newargv.push_back(lint ? "-l" : "-f");
      newargv.push_back(argv[cnt++]);
      need_file = false;
      break;
    }
    if (strcmp(argv[cnt], "-d")  == 0) {
      if (ini_fd == -1) {
        ini_fd = mkstemp(ini_path);
        if (ini_fd == -1) {
          fprintf(stderr, "Error: unable to open temporary file");
          exit(EXIT_FAILURE);
        }
        write(ini_fd, "[php]\n", 6);
      }
      auto &line = argv[cnt+1];
      write(ini_fd, line, strlen(line));
      write(ini_fd, "\n", 1);
      cnt += 2;
      continue;
    }
    if (argv[cnt][0] != '-') {
      if (show) {
        newargv.push_back("-w");
      } else {
        newargv.push_back(lint ? "-l" : "-f");
      }
      newargv.push_back(argv[cnt++]);
      need_file = false;
      break;
    }
    if (strcmp(argv[cnt], "--") == 0) {
      break;
    }
    cnt++; // skip unknown options
  }

  if (need_file) {
    char tmp[] = "/tmp/php-wrap-XXXXXX";
    int tmp_fd = mkstemp(tmp);
    if (tmp_fd == -1) {
      fprintf(stderr, "Error: unable to open temporary file");
      exit(EXIT_FAILURE);
    }
    if (program == nullptr) {
      // If the program wasn't specified on the command-line, ala' -r,
      // is no command-line parameter, read the PHP file from stdin.
      string line;
      while (getline(std::cin, line)) {
        write(tmp_fd, line.c_str(), line.length());
        write(tmp_fd, "\n", 1);
      }
    } else {
      // -r omits the braces
      write(tmp_fd, "<?\n", 3);
      write(tmp_fd, program, strlen(program));
    }
    close(tmp_fd);

    if (show) {
      newargv.push_back("-w");
    } else {
      newargv.push_back(lint ? "-l" : "-f");
    }
    newargv.push_back(tmp);
    newargv.push_back("--temp-file");
  }

  if (ini_fd != -1) {
    char arg[37];
    sprintf(arg, "-vServer.IniFile=%s", ini_path);
    newargv.push_back(arg);
  }

  if (cnt < argc && strcmp(argv[cnt], "--") == 0) cnt++;
  if (cnt < argc) {
    // There are arguments following the filename, so copy them.
    newargv.push_back("--");
    for (int i = cnt; i < argc; i++) {
      newargv.push_back(argv[i]);
    }
  }

  char *newargv_array[newargv.size() + 1];
  for (unsigned i = 0; i < newargv.size(); i++) {
    // printf("%s\n", newargv[i].data());
    newargv_array[i] = (char *)newargv[i].data();
  }
  // NULL-terminate the argument array.
  newargv_array[newargv.size()] = NULL;

  auto ret = execute_program(newargv.size(), newargv_array);

  if (ini_fd != -1) {
    unlink(ini_path);
  }

  return ret;
}

}
