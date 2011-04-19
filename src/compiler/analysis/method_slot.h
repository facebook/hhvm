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

#ifndef METHOD_SLOT
#define METHOD_SLOT
#include <util/base.h>

namespace HPHP {

typedef std::set<unsigned int> MethodSet;

class MethodSlot {
  std::string m_name;
  unsigned int m_serialNum; // unique for each method, 0 == error
  unsigned int m_callIndex; // used in calls
  unsigned int m_overloadIndex; // which instance for a given call_index
  unsigned int m_coalesces; // # of classes using this method

  // all methods declared in (any) same class as this one
  MethodSet conflictingMethods;

  public:
  MethodSlot(const std::string & name, unsigned int serialNum)
      : m_name(name), m_serialNum(serialNum), m_callIndex(0),
        m_overloadIndex(0), m_coalesces(0) {
  }

  std::string getName() const { return m_name; }
  unsigned int getCallIndex() const { return m_callIndex; }
  unsigned int getOverloadIndex() const { return m_overloadIndex; }
  bool isError() const { return m_serialNum == 0; }
  bool isFail() const { return m_callIndex== 0 || m_overloadIndex == 0; }
  std::string runObj() const; // e.g. MethodIndex(2, 3)
  std::string runObjParam() const; // e.g. "MethodIndex(2, 3) /* getId */, "
  bool operator>(const MethodSlot &y) const {
    return m_coalesces > y.m_coalesces;
  }

  static void genMethodSlot(AnalysisResultPtr) ;
  static void emitMethodSlot(CodeGenerator &, AnalysisResultPtr, bool isSystem);

  std::ostream& operator<< (std::ostream&) const;
  std::ostream& dump() const { return operator<<(std::cout); }

  private:
  MethodSlot() : m_serialNum(0), m_callIndex(0) {}
};
extern const MethodSlot *errorMethodSlot;

struct string_hash_i {
  size_t operator()(const std::string &str) const {
    // like __stl_hash_string(const char* __s)
    unsigned long h = 0;
    for (const char *s=str.c_str(); *s; ++s) {
      char c = *s;
      if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
      h = 5 * h + c;
    }
    return size_t(h);
  }
};

struct equal_to_i : public std::binary_function <std::string,std::string,bool> {
  bool operator() (const std::string& x, const std::string& y) const {
    return strcasecmp(x.c_str(), y.c_str()) == 0;
  }
};

typedef hphp_hash_map<const std::string, MethodSlot, string_hash_i, equal_to_i>
      StringToMethodSlotMap;

class CallIndexVectSet : public std::vector<MethodSet> {};

}

#endif

