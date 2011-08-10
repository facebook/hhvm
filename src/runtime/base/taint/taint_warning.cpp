/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifdef TAINTED

#include <runtime/base/complex_types.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_trace.h>
#include <runtime/base/taint/taint_warning.h>

namespace HPHP {

IMPLEMENT_REQUEST_LOCAL(TaintWarningRequestData, TaintWarning::s_requestdata);

void TaintWarning::WarnIfTainted(CStrRef s, const taint_t bit) {
  const TaintData& td = s.get()->getTaintDataRefConst();
  if (!(td.getTaint() & bit)) { return; }

  bool force_warning = false;
  std::string buf, aux;

  buf = "Using a ";
  switch (bit) {
    case TAINT_BIT_HTML:
      buf += "HTML-unsafe (tainted)";
      if (TaintTracer::IsEnabledHtml()) {
        force_warning = true;
        aux = TaintTracer::ExtractTrace(td.getTaintTrace());
      }
      break;

    case TAINT_BIT_MUTATED:
      buf += "non-static (tainted)";
      break;

    case TAINT_BIT_SQL:
      buf += "SQL-unsafe (tainted)";
      break;

    case TAINT_BIT_SHELL:
      buf += "shell-unsafe (tainted)";
      break;

    case TAINT_BIT_ALL:
      buf += "tainted";
      break;

    default:
      return;
  }
  buf += " string!\n";

  if (RuntimeOption::EnableTaintWarnings || force_warning) {
    buf += aux;
    buf += "\n";

    buf += "---begin output---\n";
    buf += s.c_str();
    buf += "\n";
    buf += "----end output----\n";

    ZeroCount(bit);
    raise_warning(buf);
  } else {
    IncCount(bit);
  }
}

}

#endif // TAINTED
