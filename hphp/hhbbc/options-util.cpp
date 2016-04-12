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
#include "hphp/hhbbc/options-util.h"

#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

bool method_map_contains(const MethodMap& mmap,
                         borrowed_ptr<const php::Class> cls,
                         borrowed_ptr<const php::Func> func) {
  std::string const clsname = cls ? cls->name->data() : "";
  auto it = mmap.find(clsname);
  if (it == end(mmap)) return false;
  return it->second.count(func->name->data());
}

bool is_trace_function(borrowed_ptr<const php::Class> cls,
                       borrowed_ptr<const php::Func> func) {
  return method_map_contains(options.TraceFunctions, cls, func);
}

bool is_interceptable_function(borrowed_ptr<const php::Class> cls,
                               borrowed_ptr<const php::Func> func) {
  if (options.AllFuncsInterceptable) return true;
  return method_map_contains(options.InterceptableFunctions, cls, func);
}

//////////////////////////////////////////////////////////////////////

}}
