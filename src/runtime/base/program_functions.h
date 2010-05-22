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

#ifndef __HPHP_PROGRAM_FUNCTIONS_H__
#define __HPHP_PROGRAM_FUNCTIONS_H__

#include <util/base.h>
#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Main entry point of the entire program.
 */
int execute_program(int argc, char **argv);

/**
 * Setting up environment variables.
 */
void process_env_variables(Variant &variables);

/**
 * Inserting a variable into specified symbol table.
 *
 * "overwrite" parameter is only for cookies:
 * According to rfc2965, more specific paths are listed above the less
 * specific ones. If we encounter a duplicate cookie name, we should
 * skip it, since it is not possible to have the same (plain text)
 * cookie name for the same path and we should not overwrite more
 * specific cookies with the less specific ones.
 */
void register_variable(Variant &variables, char *name, CVarRef value,
                       bool overwrite = true);

String canonicalize_path(CStrRef path, const char* root, int rootLen);

/**
 * Translate hex encode stack into both C++ and PHP frames.
 */
std::string translate_stack(const char *hexencoded,
                            bool with_frame_numbers = true);

time_t start_time();

///////////////////////////////////////////////////////////////////////////////
// C++ ffi

class ExecutionContext;

void hphp_process_init();
void hphp_session_init();

ExecutionContext *hphp_context_init();
bool hphp_warmup(ExecutionContext *context, const std::string &warmupDoc,
                 const std::string reqInitFunc, bool &error);
bool hphp_invoke(ExecutionContext *context, const std::string &cmd,
                 bool func, CArrRef funcParams, Variant funcRet,
                 const std::string &warmupDoc, const std::string reqInitFunc,
                 bool &error, std::string &errorMsg);
void hphp_context_exit(ExecutionContext *context, bool psp,
                       bool shutdown = true);

void hphp_session_exit();
void hphp_process_exit();
bool hphp_is_warmup_enabled();
void hphp_set_warmup_enabled();

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_PROGRAM_FUNCTIONS_H__
