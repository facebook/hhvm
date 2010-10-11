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

#include <compiler/analysis/dependency_graph.h>
#include <compiler/analysis/code_error.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/parser/parser.h>
#include <util/util.h>
#include <compiler/statement/statement.h>
#include <compiler/option.h>
#include <util/exception.h>
#include <util/logger.h>

using namespace HPHP;
using namespace HPHP::JSON;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// class Dependency

Dependency::Dependency() : m_programCount(0) {
}

///////////////////////////////////////////////////////////////////////////////
// class DependencyGraph

DependencyGraph::DependencyGraph() : m_total(0) {
  m_forwards.resize(KindOfCount);
  m_reverses.resize(KindOfCount);
  m_parents.resize(KindOfCount);
  m_allChildren.resize(KindOfCount);
  m_allParents.resize(KindOfCount);
}

void DependencyGraph::clearCache(KindOf kindOf) {
  m_allChildren[kindOf].clear();
  m_allParents[kindOf].clear();
}

///////////////////////////////////////////////////////////////////////////////

string DependencyGraph::GetIncludeFilePath(const string &source,
                                           string expText, bool documentRoot) {
  if (expText.size() <= 2) return "";

  char first = expText[0];
  char last = expText[expText.size() - 1];

  // (exp)
  if (first == '(' && last == ')') {
    expText = expText.substr(1, expText.size() - 2);
    return GetIncludeFilePath(source, expText, documentRoot);
  }

  // 'string'
  if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
    expText = expText.substr(1, expText.size() - 2);

    // absolute path
    if (!expText.empty() && expText[0] == '/') {
      return expText;
    }

    // relative path to document root
    if (documentRoot) {
      return expText;
    }

    struct stat sb;

    // relative path to containing file's directory
    ASSERT(source.size() > 1);
    size_t pos = source.rfind('/');
    string resolved;
    if (pos != string::npos) {
      resolved = source.substr(0, pos + 1) + expText;
      if (stat(resolved.c_str(), &sb) == 0) {
        return resolved;
      }
    }

    // if file cannot be found, resolve it using search paths
    for (unsigned int i = 0; i < Option::IncludeSearchPaths.size(); i++) {
      string filename = Option::IncludeSearchPaths[i] + "/" + expText;
      if (stat(filename.c_str(), &sb) == 0) {
        return filename;
      }
    }

    // try still use relative path to containing file's directory
    if (!resolved.empty()) {
      return resolved;
    }

    return expText;
  }

  // [IncludeRoot] . 'string'
  for (map<string, string>::const_iterator iter = Option::IncludeRoots.begin();
       iter != Option::IncludeRoots.end(); iter++) {
    string rootExp = iter->first + " . ";
    int rootLen = rootExp.size();
    if (expText.substr(0, rootLen) == rootExp &&
        (int)expText.length() > rootLen + 2 &&
        ((expText[rootLen] == '\'' && last == '\'') ||
         (expText[rootLen] == '"' && last == '"'))) {
      expText = expText.substr(rootLen + 1, expText.length() - rootLen - 2);

      string includeRoot = iter->second;
      if (!includeRoot.empty()) {
        if (includeRoot[0] == '/') includeRoot = includeRoot.substr(1);
        if (includeRoot.empty() ||
            includeRoot[includeRoot.size()-1] != '/') {
          includeRoot += "/";
        }
      }
      if (!expText.empty() && expText[0] == '/') {
        expText = expText.substr(1);
      }
      expText = includeRoot + expText;
      return expText;
    }
  }

  return "";
}

bool DependencyGraph::CheckInclude(std::string &included,
                                   ConstructPtr includeExp,
                                   ExpressionPtr fileExp, bool documentRoot) {
  string container = includeExp->getLocation()->file;
  included = GetIncludeFilePath(container, fileExp->getText(), documentRoot);
  included = Util::canonicalize(included);
  if (included.empty() || container == included) {
    if (!included.empty()) {
      Compiler::Error(Compiler::BadPHPIncludeFile, includeExp);
    }
    return false;
  }
  return true;
}

string DependencyGraph::add(KindOf kindOf, ConstructPtr childExp,
                            ExpressionPtr parentExp,
                            bool documentRoot /* = false */) {
  string child = childExp->getLocation()->file;
  string parent;

  if (kindOf == KindOfPHPInclude &&
      !CheckInclude(parent, childExp, parentExp, documentRoot)) {
    return "";
  }

  string program; // no program is associated
  add(kindOf, program, child, childExp, parent, ConstructPtr());
  return parent;
}

void DependencyGraph::add(KindOf kindOf, const string &childName,
                          ConstructPtr child, const string &parentName) {
  ASSERT(kindOf == KindOfClassDerivation || kindOf == KindOfFunctionCall);
  string program; // no program is associated yet
  add(kindOf, program, childName, child, parentName, ConstructPtr());
}

void DependencyGraph::add(KindOf kindOf, const string &program,
                          const string &childName, ConstructPtr child,
                          const string &parentName, ConstructPtr parent) {
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  ASSERT(!childName.empty());
  ASSERT(!parentName.empty());
  clearCache(kindOf);

  DependencyPtrMapPtr &dependencies =
    m_forwards[kindOf][parentName][childName];
  if (!dependencies) {
    dependencies.reset(new DependencyPtrMap());
    m_reverses[kindOf][childName][parentName] = dependencies;
  } else if (kindOf == KindOfPHPInclude) {
    // two identical file includes
    Compiler::Error(Compiler::RedundantInclude, child,
                    dependencies->begin()->first);
  }

  DependencyPtr &dep = (*dependencies)[child];
  if (dep) {
    // completing unresolved dependency
    if (!dep->m_parent && parent) {
      dep->m_parent = parent;
      if (Option::DependencyMaxProgram && !program.empty()) {
        dep->m_programs.push_back(program);
      }
    }
    // comparing fully resolved dependencies
    else if (dep->m_parent == parent) {
      dep->m_programCount++;
      if ((int)dep->m_programs.size() < Option::DependencyMaxProgram) {
        dep->m_programs.push_back(program);
      }
    }
  }
  // adding new dependency
  else {
    dep.reset(new Dependency());
    dep->m_parent = parent;
    dep->m_child = child;
    dep->m_programCount = 1;
    if (Option::DependencyMaxProgram && !program.empty()) {
      dep->m_programs.push_back(program);
    }
    m_total++;
  }
}

void DependencyGraph::addParent(KindOf kindOf, const std::string &program,
                                const std::string &parentName,
                                ConstructPtr parent) {
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  ASSERT(!parentName.empty());

  DependencyPtr &dep = m_parents[kindOf][parentName];
  if (!dep) {
    dep = DependencyPtr(new Dependency());
    dep->m_parent = parent;
    dep->m_programCount = 1;
    if (Option::DependencyMaxProgram && !program.empty()) {
      dep->m_programs.push_back(program);
    }
    m_parents[kindOf][parentName] = dep;
  }
}

ConstructPtr DependencyGraph::getParent(KindOf kindOf,
                                        const std::string &name) const {
  const StringToDependencyPtrMap &parents = m_parents[kindOf];
  StringToDependencyPtrMap::const_iterator iter = parents.find(name);
  if (iter != parents.end()) {
    return iter->second->m_parent;
  }
  return ConstructPtr();
}

///////////////////////////////////////////////////////////////////////////////

bool DependencyGraph::hasAnyChildren(KindOf kindOf,
                                     const std::string &parent) const {
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  const DependencyMapMap &childMapMap = m_forwards[kindOf];
  MapMapConstIter iter = childMapMap.find(parent);
  return iter != childMapMap.end();
}

const StringToConstructPtrMap&
DependencyGraph::getAllChildren(KindOf kindOf, const string &parent) const {
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  StringConstructMapMap &childMap = m_allChildren[kindOf];

  StringConstructMapMap::const_iterator iter = childMap.find(parent);
  if (iter != childMap.end()) return iter->second;

  StringToConstructPtrMap &children = childMap[parent];
  const DependencyMapMap &childMapMap = m_forwards[kindOf];
  MapMapConstIter iterMapMap = childMapMap.find(parent);
  if (iterMapMap != childMapMap.end()) {
    for (MapConstIter iterMap = iterMapMap->second.begin();
         iterMap != iterMapMap->second.end(); ++iterMap) {
      const string &child = iterMap->first;
      children[child] = iterMap->second->begin()->first;

      // this is recursively done so children can have theirs cached as well
      const StringToConstructPtrMap &grandChildren =
        getAllChildren(kindOf, child);
      children.insert(grandChildren.begin(), grandChildren.end());
    }
  }
  return children;
}

const StringToConstructPtrMap&
DependencyGraph::getAllParents(KindOf kindOf, const std::string &child) const {
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  StringConstructMapMap &parentMap = m_allParents[kindOf];

  StringConstructMapMap::const_iterator iter = parentMap.find(child);
  if (iter != parentMap.end()) return iter->second;

  StringToConstructPtrMap &parents = parentMap[child];
  const DependencyMapMap &parentMapMap = m_reverses[kindOf];
  MapMapConstIter iterMapMap = parentMapMap.find(child);
  if (iterMapMap != parentMapMap.end()) {
    for (MapConstIter iterMap = iterMapMap->second.begin();
         iterMap != iterMapMap->second.end(); ++iterMap) {
      const string &parent = iterMap->first;
      parents[parent] = iterMap->second->begin()->second->m_parent;

      // this is recursively done so parents can have theirs cached as well
      const StringToConstructPtrMap &grandParents =
        getAllParents(kindOf, parent);
      parents.insert(grandParents.begin(), grandParents.end());
    }
  }
  return parents;
}

void DependencyGraph::getChildren(KindOf kindOf, const std::string &parent,
                                  DependencyPtrVec &children) const {
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  MapMapConstIter iterMapMap = m_forwards[kindOf].find(parent);
  if (iterMapMap != m_forwards[kindOf].end()) {
    const DependencyMap &depMap = iterMapMap->second;
    for (MapConstIter iter = depMap.begin(); iter != depMap.end(); ++iter) {
      const DependencyPtrMap &dependencies = *iter->second;
      children.reserve(children.size() + dependencies.size());
      for (DepConstIter iterDeps = dependencies.begin();
          iterDeps != dependencies.end(); ++iterDeps) {
        children.push_back(iterDeps->second);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

std::vector<const char *> DependencyGraph::DependencyTexts;
std::vector<const char *> &DependencyGraph::getDependencyTexts() {
  if (DependencyTexts.empty()) {
    DependencyTexts.resize(KindOfCount);
#define DEPENDENCY_ENTRY(x) DependencyTexts[KindOf ## x] = #x;
#include "compiler/analysis/dependency.inc"
#undef DEPENDENCY_ENTRY
  }
  return DependencyTexts;
}

void Dependency::serialize(JSON::OutputStream &out) const {
  JSON::MapStream ms(out);
  if (m_parent) {
    ms.add("p", m_parent);
  }
  if (m_child) {
    ms.add("c", m_child);
  }
  ms.add("c", m_programCount)
    .add("f", m_programs)
    .done();
}

void DependencyGraph::serialize(JSON::OutputStream &out) const {
  JSON::MapStream ms(out);
  ms.add("Count", m_total);
  ms.add("ParentToChildren");
  toJSON(out, m_forwards);
  ms.add("ChildToParents");
  toJSON(out, m_reverses);
  ms.done();
}

void DependencyGraph::saveToFiles(const char *dir) const {
  vector<const char *> &dependencyTexts = getDependencyTexts();
  for (int i = 0; i < KindOfCount; i++) {
    string file = dir;
    file += "/";
    file += dependencyTexts[i];
    file += ".js";

    ofstream f(file.c_str());
    if (f) {
      JSON::OutputStream o(f);
      JSON::MapStream ms(o);
      ms.add("ParentToChildren");
      toJSON(o, m_forwards[i]);
      ms.add("ParentToChildren");
      toJSON(o, m_reverses[i]);
      ms.done();
      f.close();
    }
  }
}

void DependencyGraph::toJSON(JSON::OutputStream &out,
           const std::vector<DependencyMapMap> &depMapMapList) {
  JSON::ListStream ms(out);
  for (unsigned int i = 0; i < depMapMapList.size(); ++i) {
    ms.next();
    toJSON(out, depMapMapList[i]);
  }
  ms.done();
}

void DependencyGraph::toJSON(JSON::OutputStream &out,
                             const DependencyMapMap &depMapMap) {
  JSON::MapStream mapMapMS(out);
  for (MapMapConstIter iterMapMap = depMapMap.begin();
      iterMapMap != depMapMap.end(); ++iterMapMap) {
    const DependencyMap& depMap = iterMapMap->second;
    mapMapMS.add(iterMapMap->first);
    JSON::MapStream mapMS(out);
    for (MapConstIter iterMap = depMap.begin();
        iterMap != depMap.end(); ++iterMap) {
      const DependencyPtrMap& deps = *iterMap->second;
      mapMS.add(iterMap->first);
      JSON::ListStream depsMS(out);
      for (DepConstIter iterDeps = deps.begin();
           iterDeps != deps.end(); ++iterDeps) {
        const Dependency &dep = *iterDeps->second;
        depsMS << dep;
      }
      depsMS.done();
    }
    mapMS.done();
  }
  mapMapMS.done();
}

void DependencyGraph::saveToDB(ServerDataPtr server, int runId) const {
  vector<const char *> &dependencyTexts = getDependencyTexts();

  DBConn conn;
  conn.open(server);

  const char *sql = "INSERT INTO hphp_dep (run, program, kind, parent, "
    "parent_file, parent_line, child, child_file, child_line)";
  DBQueryPtr q(new DBQuery(&conn, sql));
  int count = 0;
  const int MAX_COUNT = 1000;

  for (int kindOf = 0; kindOf < KindOfCount; kindOf++) {
    const DependencyMapMap &mapmap = m_forwards[kindOf];
    const char *depText = dependencyTexts[kindOf];

    int k = kindOf;
    const StringToDependencyPtrMap &parents = m_parents[k];

    // non-orphaned parents
    for (MapMapConstIter iterParent = mapmap.begin();
         iterParent != mapmap.end(); ++iterParent) {
      const std::string &parent = iterParent->first;
      const DependencyMap &depMap = iterParent->second;
      for (MapConstIter iterChild = depMap.begin();
           iterChild != depMap.end(); ++iterChild) {
        const std::string &child = iterChild->first;
        const DependencyPtrMap &deps = *iterChild->second;
        for (DepConstIter iterDeps = deps.begin();
             iterDeps != deps.end(); ++iterDeps) {
          Dependency &dep = *iterDeps->second;

          const char *parentFile = "";
          const char *childFile = "";
          int parentLine = 0;
          int childLine = 0;
          StringToDependencyPtrMap::const_iterator iter =
            parents.find(parent);
          if (iter != parents.end()) {
            if (iter->second->m_parent) {
              parentFile = iter->second->m_parent->getLocation()->file;
              parentLine = iter->second->m_parent->getLocation()->line1;
            }
          } else if (dep.m_parent) {
            parentFile = dep.m_parent->getLocation()->file;
            parentLine = dep.m_parent->getLocation()->line1;
          }
          if (dep.m_child) {
            childFile = dep.m_child->getLocation()->file;
            childLine = dep.m_child->getLocation()->line1;
          }

          vector<string> programs = dep.m_programs;
          if (programs.empty()) programs.push_back("");
          for (unsigned int p = 0; p < programs.size(); p++) {
            q->insert("%d,'%s','%s', '%s','%s',%d, '%s','%s',%d",
                      runId, programs[p].c_str(), depText,
                      parent.c_str(), parentFile, parentLine,
                      child.c_str(), childFile, childLine);
            if (++count >= MAX_COUNT) {
              count = 0;
              q->execute();
              q = DBQueryPtr(new DBQuery(&conn, sql));
            }
          }
        }
      }
    }

    // orphaned parents
    for (StringToDependencyPtrMap::const_iterator iter = parents.begin();
         iter != parents.end(); ++iter) {
      if (mapmap.find(iter->first) == mapmap.end()) {
        Dependency &dep = *iter->second;

        const char *parentFile = "";
        int parentLine = 0;
        if (dep.m_parent && dep.m_parent->getLocation()) {
          parentFile = dep.m_parent->getLocation()->file;
          parentLine = dep.m_parent->getLocation()->line1;
        }

        vector<string> programs = dep.m_programs;
        if (programs.empty()) programs.push_back("");
        for (unsigned int p = 0; p < programs.size(); p++) {
          q->insert("%d,'%s','%s', '%s','%s',%d, '','',0",
                    runId, programs[p].c_str(), depText,
                    iter->first.c_str(), parentFile, parentLine);
          if (++count >= MAX_COUNT) {
            count = 0;
            q->execute();
            q = DBQueryPtr(new DBQuery(&conn, sql));
          }
        }
      }
    }
  }

  if (count) q->execute();
}

bool DependencyGraph::checkCircle(KindOf kindOf,
                                  const std::string &childName,
                                  const std::string &parentName)
{
  ASSERT(kindOf >= 0 && kindOf < KindOfCount);
  clearCache(kindOf);

  const StringToConstructPtrMap &parents = getAllParents(kindOf, parentName);
  return (parents.find(childName) != parents.end());
}
