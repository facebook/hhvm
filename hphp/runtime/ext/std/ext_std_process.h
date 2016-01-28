/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_STD_PROCESS_H_
#define incl_HPHP_EXT_STD_PROCESS_H_

#include "hphp/runtime/ext/std/ext_std.h"
#include <signal.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(shell_exec,
                      const String& cmd);
String HHVM_FUNCTION(exec,
                     const String& command,
                     VRefParam output = uninit_null(),
                     VRefParam return_var = uninit_null());
void HHVM_FUNCTION(passthru,
                   const String& command,
                   VRefParam return_var = uninit_null());
String HHVM_FUNCTION(system,
                     const String& command,
                     VRefParam return_var = uninit_null());

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(proc_open,
                      const String& cmd,
                      const Array& descriptorspec,
                      VRefParam pipes,
                      const String& cwd = null_string,
                      const Variant& env = null_variant,
                      const Variant& other_options = null_variant);
bool HHVM_FUNCTION(proc_terminate,
                   const Resource& process,
                   int signal = SIGTERM);
int64_t HHVM_FUNCTION(proc_close,
                      const Resource& process);
Array HHVM_FUNCTION(proc_get_status,
                    const Resource& process);
bool HHVM_FUNCTION(proc_nice,
                   int increment);

///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(escapeshellarg,
                     const String& arg);
String HHVM_FUNCTION(escapeshellcmd,
                     const String& command);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_STD_PROCESS_H_
