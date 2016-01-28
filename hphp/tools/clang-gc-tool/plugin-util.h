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

#ifndef incl_HPHP_CLANG_GC_TOOL_PLUGIN_UTIL_H_
#define incl_HPHP_CLANG_GC_TOOL_PLUGIN_UTIL_H_

#undef __GXX_RTTI

#include <cstdarg>
#include <set>
#include <vector>
#include <string>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <boost/format.hpp>
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>

namespace HPHP {

using DeclSet = std::set<const clang::CXXRecordDecl*>;
using DeclMap = std::map<const clang::NamedDecl*, const clang::NamedDecl*>;

template <typename C, typename K>
bool exists(const C& c, const K& k) {
  return c.find(k) != c.end();
}

/*
 * A helper class that holds an ASTContext and defines some utility
 * methods for dealing with Clang types and decls.
 */
struct PluginUtil {
  explicit PluginUtil(clang::ASTContext& context);
  ~PluginUtil();

  bool isCppFile(const char* file) const;
  bool isHeaderFile(const char* file) const;
  bool inSystemHeader(const clang::Decl* decl) const;

  // Is this decl defined in a cpp file?
  bool isHiddenDecl(const clang::CXXRecordDecl* decl) const;

  const char* getFilename(const clang::SourceRange& loc) const;

  // Get filename for the definition of decl.
  const char* getDefinitionFilename(const clang::Decl* decl) const;

  // Get filename for decl.
  const char* getFilename(const clang::Decl* decl) const;

  // Print an error message and exit.  With optional location information.
  template <typename ...Args>
  void error(const clang::SourceRange& loc,
             const char* fmt,
             Args&&... args) const {
    boost::format f(fmt);
    error(locToString(loc), format(f, std::forward<Args>(args)...));
  }
  template <typename ...Args>
  void error(const clang::Decl* decl, const char* fmt, Args&&... args) const {
    boost::format f(fmt);
    error(locToString(decl->getSourceRange()),
            format(f, std::forward<Args>(args)...));
  }
  template <typename ...Args>
  void error(const char* fmt, Args&&... args) const {
    boost::format f(fmt);
    error(std::string(""), format(f, std::forward<Args>(args)...));
  }

  // Print a warning message.  With optional location information.
  template <typename ...Args>
  void warning(const clang::Decl* declLoc,
               const char* fmt,
               Args&&... args) const {
    boost::format f(fmt);
    warning(locToString(declLoc->getSourceRange()),
              format(f, std::forward<Args>(args)...));
  }
  template <typename ...Args>
  void warning(const clang::SourceRange& loc,
               const char* fmt,
               Args&&... args) const {
    boost::format f(fmt);
    warning(locToString(loc), format(f, std::forward<Args>(args)...));
  }
  template <typename ...Args>
  void warning(const char* fmt, Args&&... args) const {
    boost::format f(fmt);
    warning(std::string(""), format(f, std::forward<Args>(args)...));
  }

  // Print a warning with a chain of reasoning.
  template <typename ...Args>
  void warning(const DeclMap& whys,
               const clang::NamedDecl* decl,
               const char* fmt,
               Args&&... args) const {
    boost::format f(fmt);
    warning(whys, decl, format(f, std::forward<Args>(args)...));
  }

  // Test for inheritance.
  static bool isSubclassOf(const clang::NamedDecl* maybeBase,
                           const clang::CXXRecordDecl* derived);

  // Is derived a subclass of any class from maybeBases?
  static bool isSubclassOf(const DeclSet& maybeBases,
                           const clang::CXXRecordDecl* derived);

  static bool isArrayType(const clang::Type& ty);
  static bool isPointerType(const clang::Type& ty);
  static bool isReferenceType(const clang::Type& ty);
  static bool isPointerOrReferenceType(const clang::Type& ty);
  static bool isPOD(const clang::Type& ty);
  bool isOpaque(const clang::Type& ty) const;

  const clang::Type& getElementType(const clang::Type& ty) const;
  const clang::Type& getPointeeType(const clang::Type& ty) const;

  // Strip pointers, references and arrays from a type.
  const clang::Type& stripType(const clang::Type& ty) const;

  const clang::CXXRecordDecl*
  getCanonicalDef(const clang::CXXRecordDecl* decl) const;

  static bool isTemplate(const clang::CXXRecordDecl* decl);

  static const clang::ClassTemplateDecl* getTemplateDef(
    const clang::CXXRecordDecl* decl);

  std::string getTemplateClassName(const clang::CXXRecordDecl* decl,
                                   bool no_namespaces = false) const;
  static void forEachTemplateParam(const clang::CXXRecordDecl* decl,
    const std::function<bool(const clang::NamedDecl*)>& fn);

  std::string getName(const clang::QualType ty,
                      bool no_namespaces = false,
                      bool suppress_tag = true,
                      bool suppress_qualifiers = true) const;

  std::string getName(const clang::Type& ty,
                      bool no_namespaces = false,
                      bool suppress_tag = true) const;

  std::string getName(const clang::CXXRecordDecl* decl,
                      bool no_namespaces = false,
                      bool suppress_tag = true) const;

  std::string getNsName(const clang::CXXRecordDecl* decl) const;

  std::string getName(const clang::ClassTemplateDecl* decl,
                      bool no_namespaces = false,
                      bool suppress_tag = true) const;

  std::string getName(const clang::NamedDecl* decl,
                      bool no_namespaces = false,
                      bool suppress_tag = true) const;

  std::string tagName(const clang::CXXRecordDecl* decl) const;

  static bool isNestedDecl(const clang::CXXRecordDecl* decl);
  static bool isNestedInTemplate(const clang::CXXRecordDecl* decl);

  template <typename F>
  static bool isNestedInFn(const clang::CXXRecordDecl* decl, F& fn) {
    auto p = decl->getDeclContext();
    while (p) {
      if (p->isRecord() && fn(clang::cast<clang::CXXRecordDecl>(p))) {
        return true;
      }
      p = p->getParent();
    }
    return false;
  }

  bool isInAnonymousNamespace(const clang::CXXRecordDecl* decl) const;

  bool isAnonymous(const clang::CXXRecordDecl* decl) const;
  bool isAnonymous(const clang::FieldDecl* decl) const;
  bool isAnonymous(const clang::NamespaceDecl* decl) const;

  static std::vector<const clang::NamespaceDecl*>
  getParentNamespaces(const clang::CXXRecordDecl* def);

  std::vector<const clang::CXXRecordDecl*>
  getOuterClasses(const clang::CXXRecordDecl* def) const;

  // Find comment (if any) immediately preceding decl.
  const clang::RawComment* findComment(const clang::Decl* decl) const;
  void collectComments();

  clang::ASTContext& m_context;
  std::set<const clang::RawComment*> m_comments;
protected:
  std::string addNamespaces(const clang::NamedDecl* decl,
                            const std::string& str,
                            bool noClasses = false) const;
  std::string addNamespaces(const clang::Type& ty,
                            const std::string& str,
                            bool noClasses = false) const;
private:
  std::string format(boost::format& f) const {
    return boost::str(f);
  }

  template <typename T, typename... Args>
  std::string format(boost::format& f, T&& v, Args&&... args) const {
    return format(f % std::forward<T>(v), std::forward<Args>(args)...);
  }

  std::string locToString(const clang::SourceRange& loc) const;
  void error(const std::string& locStr, const std::string& msg) const;
  void warning(const std::string& locStr, const std::string& msg) const;
  void warning(const DeclMap& whys,
               const clang::NamedDecl* decl,
               const std::string& msg) const;
};

// Simple file locking class.
struct Lockfile {
  explicit operator bool() const { return m_fd != -1 && m_flockres == 0; }
  void unlock() {
    if (m_fd >= 0) {
      flock(m_fd, LOCK_UN);
      close(m_fd);
      int res = unlink(m_filename.c_str());
      (void)res;
      assert(res == 0 || !m_exclusive);
      m_fd = -1;
    }
  }
  // excl requests an exclusive lock.
  explicit Lockfile(const std::string& basefile, bool exclusive = true)
    : m_filename(lockfileName(basefile)),
      m_exclusive(exclusive),
      m_fd(open(m_filename.c_str(), O_CREAT | O_EXCL)) {
    if (m_fd < 0 && !m_exclusive) {
      m_fd = open(m_filename.c_str(), O_RDONLY);
    }
    m_flockres = m_fd < 0 ? -1 : flock(m_fd, LOCK_EX);
  }
  ~Lockfile() {
    unlock();
  }
 private:
  Lockfile(const Lockfile&) = delete;
  Lockfile& operator=(const Lockfile&) = delete;
  static std::string lockfileName(const std::string& base) {
    auto ind = base.find_last_of('/');
    if (ind != std::string::npos) {
      return base.substr(0,ind+1) + "." + base.substr(ind+1) + ".lock";
    }
    return std::string(".") + base + ".lock";
  }
  std::string m_filename;
  bool m_exclusive;
  int m_fd;
  int m_flockres;
};

}

#endif
