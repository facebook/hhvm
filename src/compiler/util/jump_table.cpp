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
#include <compiler/util/jump_table.h>
#include <compiler/option.h>
#include <util/util.h>

namespace HPHP {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

JumpTable::JumpTable(CodeGenerator &cg,
                     const vector<const char*> &keys, bool caseInsensitive,
                     bool hasPrehash, bool useString)
  : m_cg(cg), m_subIter(0) {
  if (keys.empty()) {
    m_iter = m_table.end();
    return;
  }
  int tableSize = Util::roundUpToPowerOfTwo(keys.size() * 2);
  CodeGenerator::BuildJumpTable(keys, m_table, tableSize, caseInsensitive);
  if (hasPrehash) {
    m_cg_printf("if (hash < 0) ");
  } else {
    m_cg_printf("int64 ");
  }
  if (useString) {
    m_cg_printf("hash = StringData::Hash(s.get());\n");
  } else {
    m_cg_printf("hash = hash_string(s);\n");
  }
  m_cg.printStartOfJumpTable(tableSize);
  m_iter = m_table.begin();
  if (ready()) {
    m_cg_indentBegin("case %d:\n", m_iter->first);
  }
}

void JumpTable::next() {
  ASSERT(ready());
  m_subIter++;
  if (m_subIter >= m_iter->second.size()) {
    m_cg_indentEnd("  break;\n");
    m_subIter = 0;
    ++m_iter;
    if (m_iter == m_table.end()) {
      m_cg_printf("default:\n");
      m_cg_printf("  break;\n");
      m_cg_indentEnd("}\n");
    } else {
      m_cg_indentBegin("case %d:\n", m_iter->first);
    }
  }
}

bool JumpTable::ready() const {
  return m_iter != m_table.end();
}

const char *JumpTable::key() const {
  return m_iter->second.at(m_subIter);
}

///////////////////////////////////////////////////////////////////////////////
}
