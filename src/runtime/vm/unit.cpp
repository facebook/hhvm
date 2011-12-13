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

#include <sys/mman.h>

#include <iostream>
#include <iomanip>

#include <util/lock.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/vm/bytecode.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

Unit::Unit(const Location* sLoc) :
  m_writeDepth(0),
  m_bc((uchar*)malloc(BCMaxInit)),
  m_bclen(0),
  m_bcmax(BCMaxInit),
  m_md5(),
  m_filepath(NULL),
  m_dirpath(NULL) {
  StringData* name = StringData::GetStaticString("");
  m_main = newFunc(name, false);
  m_main->init(sLoc, 0, AttrNone, false, name);
}

Unit::~Unit() {
  unprotect();
  if (debug) {
    // poison released bytecode
    memset(m_bc, 0xff, m_bcmax);
  }
  free(m_bc);

  // Delete all Func's.
  for (std::vector<Func*>::const_iterator it = m_funcs.begin();
       it != m_funcs.end(); ++it) {
    delete *it;
  }
  // m_classes' dtor will destroy all Class'es.  ExecutionContext and the
  // TC may retain references, so it is possible for Class'es to outlive
  // their Unit.
}

// In debug builds, ensure we don't change fully emitted/optimized
// bytecode.
void Unit::protect() const {
  if (debug) {
    mprotect(m_bc, m_bcmax, PROT_READ);
  }
}

void Unit::unprotect() const {
  if (debug) {
    mprotect(m_bc, m_bcmax, PROT_READ | PROT_WRITE);
  }
}

void Unit::close() {
  // To be called when emission is complete.
  protect();
}

void Unit::replaceBc(uchar* bc, size_t bclen) {
  ASSERT(bclen <= m_bcmax);
  unprotect();
  free(m_bc);
  m_bc = bc;
  m_bclen = bclen;
  close();
}

PreClass* Unit::newPreClass(StringData* n, Attr attrs, StringData* parent,
                            StringData* docComment, const Location* sLoc,
                            Offset o, bool hoistable) {
  PreClass* preClass;
  // A class declaration is hoisted if all of the following are true:
  // 1) It is at the top level of pseudomain (as indicated by the 'hoistable'
  //    parameter).
  // 2) It is the first hoistable declaration for the class name within the
  //    unit.
  // 3) Its parent (if any) has already been defined by the time the attempt is
  //    made to hoist the class.
  // Only the first two conditions are enforced here, because (3) cannot be
  // precomputed.
  if (hoistable && m_hoistablePreClassSet.find(n) ==
      m_hoistablePreClassSet.end()) {
    preClass = new PreClass(this, sLoc, o, n, attrs, parent, docComment,
                            m_preClasses.size(), true);
    m_hoistablePreClassSet.insert(n);
    m_hoistablePreClassVec.push_back(preClass);
  } else {
    preClass = new PreClass(this, sLoc, o, n, attrs, parent, docComment,
                            m_preClasses.size(), false);
  }
  m_preClasses.push_back(PreClassPtr(preClass));
  return preClass;
}

Func* Unit::newFunc(StringData* n, bool top) {
  Func* func = new Func(*this, m_funcs.size(), n);
  m_funcs.push_back(func);
  if (top) {
    if (m_funcMap.find(n) != m_funcMap.end()) {
      raise_error("Function already defined: %s", n->data());
    }
    m_funcMap[n] = func;
  }
  return func;
}

std::string Unit::toString() const {
  std::ostringstream ss;
  prettyPrint(ss);
  for (std::vector<PreClassPtr>::const_iterator it = m_preClasses.begin();
      it != m_preClasses.end(); ++it) {
    (*it).get()->prettyPrint(ss);
  }
  for (std::vector<Func*>::const_iterator it = m_funcs.begin();
      it != m_funcs.end(); ++it) {
    (*it)->prettyPrint(ss);
  }
  return ss.str();
}

Id Unit::mergeLitstr(StringData* litstr, bool isAnon /* = false */) {
  hphp_hash_map<const StringData*, Id,
                string_data_hash, string_data_same>::iterator
    it = m_litstr2id.find(litstr);
  if (it == m_litstr2id.end()) {
    StringData* sd = (isAnon) ? litstr : StringData::GetStaticString(litstr);

    Id id = m_litstrs.size();
    m_litstrs.push_back(sd);
    m_litstr2id[sd] = id;
    return id;
  } else {
    return it->second;
  }
}

StringData* Unit::lookupLitstrStr(StringData* litstr) {
  StringData* sd = StringData::GetStaticString(litstr);
  mergeLitstr(sd, true);
  return sd;
}

StringData* Unit::lookupLitstrStr(const std::string& str) {
  StringData* litstr = StringData::GetStaticString(str);
  mergeLitstr(litstr, true);
  return litstr;
}

StringData* Unit::lookupLitstrId(Id id) const {
  return (StringData*)m_litstrs.at(id);
}

// Global collection of static arrays.  Only one copy of each unique array is
// ever stored, as a result of using this collection.
Mutex s_ArraysLock;
hphp_hash_map<const StringData*, ArrayData*,
              string_data_hash, string_data_same> s_Arrays;

ArrayData* Unit::mergeAnonArray(ArrayData* a,
                                const StringData* key /* = NULL */) {
  String s;
  if (key == NULL) {
    s = f_serialize(a);
    key = StringData::GetStaticString(s.get());
  }

  Lock lock(s_ArraysLock);
  hphp_hash_map<const StringData*, ArrayData*,
                string_data_hash, string_data_same>::iterator
    it = s_Arrays.find(key);
  if (it == s_Arrays.end()) {
    // Copy to avoid SmartAllocator
    ASSERT(IsHphpArray(a));
    HphpArray* copy = new HphpArray(a->size());
    static_cast<HphpArray*>(a)->copyTo(copy);
    copy->setStatic();
    copy->onSetStatic();
    s_Arrays[key] = copy;
    return copy;
  } else {
    return it->second;
  }
}

Id Unit::mergeArray(ArrayData* a) {
  String s = f_serialize(a);
  const StringData* key = StringData::GetStaticString(s.get());

  hphp_hash_map<const StringData*, Id,
                string_data_hash, string_data_same>::iterator
    it = m_array2id.find(key);
  if (it == m_array2id.end()) {
    a = Unit::mergeAnonArray(a, key);

    Id id = m_arrays.size();
    m_arrays.push_back(a);
    m_array2id[key] = id;
    return id;
  } else {
    return it->second;
  }
}

ArrayData* Unit::lookupArrayId(Id id) const {
  return m_arrays.at(id);
}

template<typename T>
static void addToIntervalMap(std::map<Offset, IntervalMapEntry<T> > *intMap,
                             Offset start, Offset end,
                             IntervalMapEntry<T> &entry) {
  typename std::map<Offset, IntervalMapEntry<T> >::iterator it =
    intMap->lower_bound(start);
  if (it != intMap->end() && it->second.val == entry.val) {
    entry.startOffset = it->second.startOffset;
    intMap->erase(start);
  } else {
    entry.startOffset = start;
  }
  entry.endOffset = end;
  (*intMap)[end] = entry;
}

void Unit::recordSourceLocation(const Location *sLoc, Offset start,
                                Offset end) {
  ASSERT(sLoc);

  int lineNum = sLoc->line1;
  // Only add a new entry if we've moved onto a different line.
  if (m_lines.empty() || lineNum != m_lines.back()) {
    // m_lineRuns must be in nondecreasing order.
    ASSERT((m_lineRuns.empty() && start == 0) || end >= m_lineRuns.back());
    m_lineRuns.push_back(end);
    m_lines.push_back(lineNum);
  } else {
    m_lineRuns.back() = end;
  }

  SourceLocEntry newEntryLoc;
  newEntryLoc.val.setLoc(sLoc);
  addToIntervalMap(&m_sourceLocTable, start, end, newEntryLoc);
}

void Unit::recordFunction(Func *func) {
  FuncEntry newEntry;
  newEntry.val = func;
  addToIntervalMap(&m_funcTable, func->m_base, func->m_past, newEntry);
}

int Unit::getLineNumber(Offset pc) const {
  std::vector<Offset>::const_iterator it =
    upper_bound(m_lineRuns.begin(), m_lineRuns.end(), pc);

  if (it != m_lineRuns.end()) {
    ASSERT(*it > pc);
    int index = it - m_lineRuns.begin();
    ASSERT(index >= 0 && index < (int)m_lines.size());
    return m_lines[index];
  }
  return -1;
}

const SourceLoc* Unit::getSourceLoc(Offset pc) {
  std::map<Offset, SourceLocEntry>::iterator
    it = m_sourceLocTable.upper_bound(pc);
  if (it != m_sourceLocTable.end()) {
    return &it->second.val;
  }
  return NULL;
}

Func* Unit::getFunc(Offset pc) const {
  std::map<Offset, FuncEntry>::const_iterator
    it = m_funcTable.upper_bound(pc);
  if (it != m_funcTable.end()) {
    return it->second.val;
  }
  return NULL;
}

void Unit::dumpUnit(Unit* u) {
  std::cerr << u->toString();
}

void Unit::prettyPrint(std::ostream &out) const {
  uchar* it = m_bc;

  for (unsigned i = 0; i < m_lineRuns.size(); ++i) {
    out << "// line " << m_lines[i] << std::endl;

    while (it < &m_bc[m_lineRuns[i]]) {
      out << std::setw(4) << (it - m_bc) << ": ";
      out << instrToString((Opcode*)it, (Unit*)this) << std::endl;
      it += instrLen((Opcode*)it);
    }
  }
}

Class* Unit::defClass(PreClass* preClass, bool failIsFatal) {
  Lock lock(m_classesMutex);
  // If no classes of this name are defined, this will create an empty
  // vector as a side effect.
  std::vector<ClassPtr>& classes = m_classes[preClass->m_name];
  // Search for a compatible extant class.  Searching from most to least
  // recently created may have better locality than alternative search orders.
  for (uint i = classes.size(); i--; ) {
    Class* class_ = classes[i].get();
    switch (class_->equiv(preClass, failIsFatal /*tryAutoload*/)) {
    case Class::EquivFalse: {
      break;
    }
    case Class::EquivTrue: {
      return class_;
    }
    case Class::EquivFail: {
      ASSERT(!failIsFatal);
      return NULL;
    }
    default: ASSERT(false);
    }
  }
  // Create a new class and cache it.
  Class* parent;
  if (preClass->m_parent->size() != 0) {
    parent = g_context->getClass(preClass->m_parent, failIsFatal);
    if (parent == NULL) {
      if (failIsFatal) {
        raise_error("Undefined class: %s", preClass->m_parent->data());
      }
      return NULL;
    }
  } else {
    parent = NULL;
  }
  ClassPtr class_(Class::newClass(preClass, parent, failIsFatal));
  if (class_.get() == NULL) {
    ASSERT(!failIsFatal);
    return NULL;
  }
  classes.push_back(class_);
  return class_.get();
}

///////////////////////////////////////////////////////////////////////////////
}
}
