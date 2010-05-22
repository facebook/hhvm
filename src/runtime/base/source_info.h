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

#ifndef __HPHP_SOURCE_INFO_H__
#define __HPHP_SOURCE_INFO_H__

#include <util/stack_trace.h>
#include <util/mutex.h>
#include <util/case_insensitive.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
class SourceInfoHook;

class SourceInfo {
public:
  static SourceInfo TheSourceInfo;

public:
  SourceInfo();

  /**
   * Load from static variables.
   */
  void load();

  /**
   * Translate a C++ frame to PHP frame.
   */
  bool translate(StackTrace::FramePtr f);

  /**
   * Returns a list of functions declared in specified file.
   */
  void getDeclaredFunctions(const char *filename,
                            std::vector<const char *> &functions);

  /**
   * Returns a list of classes declared in specified file.
   */
  void getDeclaredClasses(const char *filename,
                          std::vector<const char *> &classes);

  /**
   * Which file contains this class/function's declaration.
   */
  const char *getClassDeclaringFile(const char *name);
  const char *getFunctionDeclaringFile(const char *name);

  static void SetHook(SourceInfoHook *hook) { s_hook = hook; }

private:
  Mutex m_mutex;
  bool m_loaded;
  int m_source_root_len;

  struct LocationInfo {
    std::string file;
    int line;
  };

  typedef hphp_const_char_map<LocationInfo *> LocationMap;
  typedef hphp_const_char_map<std::vector<const char *> > NameMap;
  typedef hphp_const_char_imap<std::vector<const char *> > INameMap;

  // "file:line" in C++ code => (file, line) in PHP code
  LocationMap m_cpp2php;

  // "php source file" <=> "func/class name"
  INameMap m_cls2file;
  NameMap m_file2cls;
  INameMap m_func2file;
  NameMap m_file2func;

  void loadImpl(LocationMap &dest, const char **p);
  void loadImpl(INameMap &forward, NameMap &backward, const char **p);

  static SourceInfoHook *s_hook;
};

/**
 * Interface for a hook into source info for eval.
 */
class SourceInfoHook {
public:
  virtual ~SourceInfoHook() {}
  virtual const char *getClassDeclaringFile(const char *name) = 0;
  virtual const char *getFunctionDeclaringFile(const char *name) = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SOURCE_INFO_H__
