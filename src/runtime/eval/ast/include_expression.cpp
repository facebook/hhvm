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

#include <runtime/eval/ast/include_expression.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/runtime/eval_state.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

IncludeExpression::IncludeExpression(EXPRESSION_ARGS, bool include, bool once,
                                     ExpressionPtr file)
  : Expression(EXPRESSION_PASS), m_file(file), m_include(include), m_once(once)
{
  m_localDir = loc()->file;
  size_t fileIdx = m_localDir.rfind('/');
  if (fileIdx == string::npos) {
    m_localDir = "";
  } else {
    m_localDir = m_localDir.substr(0, fileIdx);
  }
}

Variant IncludeExpression::eval(VariableEnvironment &env) const {
  String file(m_file->eval(env).toString());
  Variant ret;
  if (m_include) {
    ret = HPHP::include(file, m_once, &env, m_localDir.c_str());
  } else {
    ret = HPHP::require(file, m_once, &env, m_localDir.c_str());
  }

  if (same(ret, true)) {
    Array &arr = RequestEvalState::GetIncludes();
    if (arr["files"][file].isNull()) {
      arr.lvalAt("files").set(file, true);

      Variant &included = arr.lvalAt("included");
      if (included.isNull()) {
        included.append(m_loc.file);
      }
      included.append(file);

      Array info;
      info.set("operation", "include");
      info.set("op_type", 2);
      info.set("filename", file);
      info.set("opened_path", file);
      info.set("fromfile", m_loc.file);
      info.set("fromline", m_loc.line0);

      Variant &inclued = arr.lvalAt("inclued").lvalAt("includes");
      inclued.append(info);
    }
  }

  return ret;
}

void IncludeExpression::dump(std::ostream &out) const {
  out << (m_include ? "include" : "require") << (m_once ? "_once" : "") << " ";
  m_file->dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

