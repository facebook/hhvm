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

#ifndef incl_HPHP_LABEL_SCOPE_H_
#define incl_HPHP_LABEL_SCOPE_H_

#include <string>
#include <vector>

#include "hphp/util/deprecated/declare-boost-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Statement);

class LabelScope {
public:
  class LabelInfo {
  public:
    LabelInfo(StatementPtr s, const std::string& name)
      : m_stmt(s), m_name(name) {}

    StatementPtr getStatement() const { return m_stmt; }
    const std::string& getName() const { return m_name; }

  private:
    StatementPtr m_stmt;
    std::string m_name;
  };

  const std::vector<LabelInfo>& getLabels() const { return m_labels; }
  void addLabel(StatementPtr s, const std::string& label) {
    m_labels.push_back(LabelInfo(s, label));
  }

private:
  std::vector<LabelInfo> m_labels;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_LABEL_SCOPE_H_
