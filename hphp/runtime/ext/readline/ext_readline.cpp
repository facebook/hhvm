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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/util/lock.h"

#ifdef USE_EDITLINE
#include <editline/readline.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace HPHP {

namespace {
struct ReadlineVars {
  Variant completion;
  Array array;
};

IMPLEMENT_THREAD_LOCAL(ReadlineVars, s_readline);

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
    iter = s_readline->array.begin();
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

static char** readline_completion_cb(const char* text, int start, int end) {
  if (!s_readline->completion.isInitialized()) {
    return nullptr;
  }
  char** matches = nullptr;
  auto completion = vm_call_user_func(
      s_readline->completion,
      make_packed_array(text, start, end));
  if (completion.isArray()) {
    s_readline->array = completion.toArrRef();
    if (s_readline->array.length() > 0) {
      matches = rl_completion_matches(text, _readline_command_generator);
    } else {
      // readline frees this using free(), so we must use malloc() and not new
      matches = (char**)malloc(sizeof(char*) * 2);
      matches[0] = strdup("");
      matches[1] = "\0";
    }
  }
  s_readline->array.clear();
  return matches;
}

static bool HHVM_FUNCTION(
    readline_completion_function,
    const Variant& function) {
  if (!is_callable(function)) {
    raise_warning(
        "readline_completion_function(): %s is not callable",
        function.toString().data());
    return false;
  }
  s_readline->completion = function;
  rl_attempted_completion_function = readline_completion_cb;
  return true;
}

static bool HHVM_FUNCTION(readline_read_history,
                          const Variant& filename /* = null */) {
  if (filename.isNull()) {
    return read_history(nullptr) == 0;
  } else {
    auto const filenameString = filename.toString();
    return read_history(filenameString.data()) == 0;
  }
}

static inline const char* convert_null_to_empty(const char* str) {
  if (str) {
    return str;
  } else {
    return "";
  }
}

const StaticString
  s_line_buffer("line_buffer"),
  s_point("point"),
  s_end("end"),
  s_mark("mark"),
  s_done("done"),
  s_pending_input("pending_input"),
  s_prompt("prompt"),
  s_terminal_name("terminal_name"),
  s_erase_empty_line("erase_empty_line"),
  s_library_version("library_version"),
  s_readline_name("readline_name"),
  s_attempted_completion_over("attempted_completion_over");

static char* _rl_readline_name = nullptr;
static char* _rl_line_buffer = nullptr;
static Mutex info_lock;

Variant HHVM_FUNCTION(readline_info, const Variant& varnameMixed /* = null */,
                      const Variant& newvalueMixed /* = null */) {
  if (varnameMixed.isNull()) {
    ArrayInit ret(12, ArrayInit::Map{});
    ret.add(s_line_buffer, convert_null_to_empty(rl_line_buffer));
    ret.add(s_point, rl_point);
    ret.add(s_end, rl_end);
#ifndef USE_EDITLINE
    ret.add(s_mark, rl_mark);
    ret.add(s_done, rl_done);
    ret.add(s_pending_input, rl_pending_input);
    ret.add(s_prompt, convert_null_to_empty(rl_prompt));
    ret.add(s_terminal_name, convert_null_to_empty(rl_terminal_name));
#endif
#if HAVE_ERASE_EMPTY_LINE
    ret.add(s_erase_empty_line, rl_erase_empty_line);
#endif
    ret.add(s_library_version, convert_null_to_empty(rl_library_version));
    ret.add(s_readline_name, convert_null_to_empty(rl_readline_name));
    ret.add(s_attempted_completion_over, rl_attempted_completion_over);
    return ret.toArray();
  } else {
    auto const varname = varnameMixed.toString();
    Variant oldval;
    if (varname == s_line_buffer) {
      oldval = String(convert_null_to_empty(rl_line_buffer));
      if (!newvalueMixed.isNull() &&
          oldval.toString() != newvalueMixed.toString()) {
        Lock lock(info_lock);
        raise_warning(
          "This probably isn't doing what you expect it to do, " \
          "this buffer is set for EVERY request."
        );
        free(_rl_line_buffer);
        auto const newvalue = newvalueMixed.toString();
        _rl_line_buffer = strdup(newvalue.data());
        rl_line_buffer = _rl_line_buffer;
      }
      return oldval;
    } else if (varname == s_point) {
      return rl_point;
    } else if (varname == s_end) {
      return rl_end;
#ifndef USE_EDITLINE
    } else if (varname == s_mark) {
      return rl_mark;
    } else if (varname == s_done) {
      oldval = rl_done;
      if (!newvalueMixed.isNull()) {
        rl_done = newvalueMixed.toInt64();
      }
      return oldval;
    } else if (varname == s_pending_input) {
      oldval = rl_pending_input;
      if (!newvalueMixed.isNull()) {
        rl_pending_input = newvalueMixed.toInt64();
      }
      return oldval;
    } else if (varname == s_prompt) {
      return convert_null_to_empty(rl_prompt);
#endif
#if HAVE_ERASE_EMPTY_LINE
    } else if (varname == s_erase_empty_line) {
      oldval = rl_erase_empty_line;
      if (!newvalueMixed.isNull()) {
        rl_erase_empty_line = newvalueMixed.toInt64();
      }
      return oldval;
#endif
    } else if (varname == s_library_version) {
      return convert_null_to_empty(rl_library_version);
    } else if (varname == s_readline_name) {
      oldval = String(convert_null_to_empty(rl_readline_name));
      if (!newvalueMixed.isNull() &&
          oldval.toString() != newvalueMixed.toString()) {
        Lock lock(info_lock);
        raise_warning(
          "This probably isn't doing what you expect it to do, " \
          "this name is set for EVERY request."
        );
        free(_rl_readline_name);
        auto const newvalue = newvalueMixed.toString();
        _rl_readline_name = strdup(newvalue.data());
        rl_readline_name = _rl_readline_name;
      }
      return oldval;
    } else if (varname == s_attempted_completion_over) {
      oldval = rl_attempted_completion_over;
      if (!newvalueMixed.isNull()) {
        rl_attempted_completion_over = newvalueMixed.toInt64();
      }
      return oldval;
    }
  }
  return null_variant;
}


static bool HHVM_FUNCTION(readline_write_history,
                          const Variant& filename /* = null */) {
  if (filename.isNull()) {
    return write_history(nullptr) == 0;
  } else {
    auto const filenameString = filename.toString();
    return write_history(filenameString.data()) == 0;
  }
}

static class ReadlineExtension final : public Extension {
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
      HHVM_FE(readline_info);
      HHVM_FE(readline_read_history);
      HHVM_FE(readline_write_history);
      loadSystemlib();
    }

    void requestShutdown() override {
      if (!s_readline.isNull()) {
        s_readline->completion.releaseForSweep();
        s_readline->array.detach();
      }
    }
} s_readline_extension;

} // namespace HPHP
