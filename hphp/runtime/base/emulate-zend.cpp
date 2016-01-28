/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/ini-setting.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

namespace HPHP {

int execute_program(int argc, char **argv);

bool check_option(const char *option) {
#ifndef _MSC_VER
  // Parameters that can be directly passed through to hhvm.
  static const char *passthru[] = {
  };

  for (int i = 0; i < sizeof(passthru) / sizeof(const char *); i++) {
    if (strcmp(option, passthru[i]) == 0) return true;
  }
#endif
  return false;
}

static int get_tempfile_if_not_exists(int ini_fd, char ini_path[]) {
  if (ini_fd == -1) {
#ifdef _MSC_VER
    // MSVC doesn't require the characters to be the last
    // 6 in the string.
    ini_fd = open(mktemp(ini_path), O_RDWR | O_EXCL);
#else
    ini_fd = mkstemps(ini_path, 4); // keep the .ini suffix
#endif
    if (ini_fd == -1) {
      fprintf(stderr, "Error: unable to open temporary file");
      exit(EXIT_FAILURE);
    }
  }
  return ini_fd;
}

int emulate_zend(int argc, char** argv) {
  std::vector<std::string> newargv;

  newargv.push_back(argv[0]);

  bool lint = false;
  bool show = false;
  bool need_file = true;
  int ini_fd = -1;
  char ini_path[] = "/tmp/php-ini-XXXXXX.ini";
  std::string ini_section = "";
  const char* program = nullptr;

  int cnt = 1;
  bool ignore_default_configs = false;
  while (cnt < argc) {
    if (check_option(argv[cnt])) {
      newargv.push_back(argv[cnt++]);
      continue;
    }
    if (strcmp(argv[cnt], "-a") == 0 ||
        strcmp(argv[cnt], "--interactive") == 0) {
      need_file = false;
      newargv.push_back("-a");
      cnt++;
      continue;
    }
    if (strcmp(argv[cnt], "-z") == 0) {
      std::string arg = "-vDynamicExtensions.0=";
      arg.append(argv[cnt+1]);
      newargv.push_back(arg.c_str());
      cnt += 2;
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
    if (strcmp(argv[cnt], "--modules") == 0) {
    // zend has a -m flag but we're already using it for --mode
      newargv.push_back("--modules");
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
    if (strcmp(argv[cnt], "-n") == 0) {
      ignore_default_configs = true;
      cnt++;
      newargv.push_back("--no-config");
      continue;
    }
    if (strcmp(argv[cnt], "-c")  == 0) {
      if (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
        newargv.push_back("-c");
        newargv.push_back(argv[cnt + 1]);
        cnt = cnt + 2;
        continue;
      } else {
        fprintf(stderr, "Notice: No config file specified");
        exit(EXIT_FAILURE);
      }
    }
    if (strcmp(argv[cnt], "-d")  == 0 || strcmp(argv[cnt], "--define") == 0) {
      ini_fd = get_tempfile_if_not_exists(ini_fd, ini_path);

      std::string line = argv[cnt+1];
      std::string section = "php";
      int pos_period = line.find_first_of('.');
      int pos_equals = line.find_first_of('=');

      if (pos_period != std::string::npos &&
          pos_equals != std::string::npos &&
          pos_period < pos_equals) {
        section = line.substr(0, pos_period);
      }

      if (section != ini_section) {
        ini_section = section;
        write(ini_fd, "[", 1);
        write(ini_fd, section.c_str(), section.length());
        write(ini_fd, "]\n", 2);
      }

      write(ini_fd, line.c_str(), line.length());
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
      std::string line;
      while (std::getline(std::cin, line)) {
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
    newargv.push_back("-c");
    newargv.push_back(ini_path);
  }

  if (ignore_default_configs) {
    // Appending empty file to -c options to avoid loading defaults
    ini_fd = get_tempfile_if_not_exists(ini_fd, ini_path);
  } else {
    // Should only include this default if not explicitly ignored.
#ifdef PHP_DEFAULT_HDF
    newargv.push_back("-c");
    newargv.push_back(PHP_DEFAULT_HDF);
#endif

    // If the -c option is specified without a -n, php behavior is to
    // load the default ini/hdf
    auto cb = [&newargv] (const char *filename) {
      newargv.push_back("-c");
      newargv.push_back(filename);
    };
    add_default_config_files_globbed(DEFAULT_CONFIG_DIR "/php*.ini", cb);
    add_default_config_files_globbed(DEFAULT_CONFIG_DIR "/config*.hdf", cb);
  }

  if (cnt < argc && strcmp(argv[cnt], "--") == 0) cnt++;
  if (cnt < argc) {
    // There are arguments following the filename, so copy them.
    newargv.push_back("--");
    for (int i = cnt; i < argc; i++) {
      newargv.push_back(argv[i]);
    }
  }

  char** newargv_array = (char**)alloca(sizeof(char*) * (newargv.size() + 1));
  for (unsigned i = 0; i < newargv.size(); i++) {
    // printf("%s\n", newargv[i].data());
    newargv_array[i] = (char *)newargv[i].data();
  }
  // NULL-terminate the argument array.
  newargv_array[newargv.size()] = nullptr;

  auto ret = execute_program(newargv.size(), newargv_array);

  if (ini_fd != -1) {
    unlink(ini_path);
  }

  return ret;
}

}
