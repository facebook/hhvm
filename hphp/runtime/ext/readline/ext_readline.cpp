/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_function.h"

#ifdef USE_EDITLINE
#include <editline/readline.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace HPHP {

namespace {
  Variant* _readline_completion;
  Array _readline_array;
}

static Variant HHVM_FUNCTION(readline, const String& prompt) {
  auto result = readline(prompt.data());
  if (result == nullptr) {
    return false;
  } else {
    auto str = String::FromCStr(result);
    free(result);
    return str;
  }
}

static bool HHVM_FUNCTION(readline_add_history, const String& line) {
  add_history(line.data());
  return true;
}

static bool HHVM_FUNCTION(readline_clear_history) {
#ifdef USE_EDITLINE
  // libedit needs this to ensure correct allocation
  using_history();
#endif
  clear_history();
  return true;
}

/* This function is called repeatedly by rl_completion_matches. The first
 * invocation has state = 0, subsequent ones have state != 0. A return value of
 * nullptr indicates that all possible completions have been returned. */
static char* _readline_command_generator(const char* text, int state) {
  static ArrayIter iter;
  if (state == 0) {
    iter = _readline_array.begin();
  }
  auto text_str = String(text);
  while (iter) {
    auto value = iter.secondRef().toString();
    ++iter;
    if (text_str == value.substr(0, text_str.length())) {
      // readline frees this using free(), so we must use malloc() and not new
      return strdup(value.data());
    }
  }
  return nullptr;
}

static char** _readline_completion_cb(const char* text, int start, int end) {
  if (_readline_completion == nullptr) {
    return nullptr;
  }
  char** matches = nullptr;
  auto completion = vm_call_user_func(
      *_readline_completion,
      make_packed_array(text, start, end));
  if (completion.isArray()) {
    _readline_array = completion.asArrRef();
    if (_readline_array.length() > 0) {
      matches = rl_completion_matches(text, _readline_command_generator);
    } else {
      // readline frees this using free(), so we must use malloc() and not new
      matches = (char**)malloc(sizeof(char*) * 2);
      matches[0] = strdup("");
      matches[1] = "\0";
    }
  }
  _readline_array.clear();
  return matches;
}

static bool HHVM_FUNCTION(
    readline_completion_function,
    const Variant& function) {
  if (!f_is_callable(function)) {
    raise_warning(
        "readline_completion_function(): %s is not callable",
        function.toString().data());
    return false;
  }
  if (_readline_completion != nullptr) {
    delete _readline_completion;
  }
  _readline_completion = new Variant(function);
  rl_attempted_completion_function = _readline_completion_cb;
  return true;
}

static bool HHVM_FUNCTION(readline_read_history,
                          const String& filename /* = null */) {
  if (filename.isNull()) {
    return read_history(NULL) == 0;
  } else {
    return read_history(filename.data()) == 0;
  }
}

static bool HHVM_FUNCTION(readline_write_history,
                          const String& filename /* = null */) {
  if (filename.isNull()) {
    return write_history(NULL) == 0;
  } else {
    return write_history(filename.data()) == 0;
  }
}

static class ReadlineExtension : public Extension {
  public:
    ReadlineExtension() : Extension("readline") {}
    void moduleInit() override {
#ifdef USE_EDITLINE
      Native::registerConstant<KindOfStaticString>(
          makeStaticString("READLINE_LIB"), makeStaticString("libedit")
      );
#else
      Native::registerConstant<KindOfStaticString>(
          makeStaticString("READLINE_LIB"), makeStaticString("readline")
      );
#endif
      HHVM_FE(readline);
      HHVM_FE(readline_add_history);
      HHVM_FE(readline_clear_history);
      HHVM_FE(readline_completion_function);
      HHVM_FE(readline_read_history);
      HHVM_FE(readline_write_history);
      loadSystemlib();
    }
} s_readline_extension;

} // namespace HPHP
