/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef HPHP_FILEBRAND_H
#define HPHP_FILEBRAND_H

#include <fstream>
#include <string>

namespace HPHP { namespace FileBrand {

void makeInvocationTrace(std::string& invocation_trace,
                         int argc,
                         const char* argv[]);

void brandOutputFile(std::ostream& out,
                     const char* short_name,
                     const std::string& invocation_trace);

}}

#endif
