/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_EXECUTION_STACK
#define incl_HPHP_ZEND_EXECUTION_STACK

#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_types.h"
#include "hphp/runtime/base/request-local.h"

enum class ZendStackMode {
  HHVM_STACK,
  SIDE_STACK
};

struct ZendStackEntry {
  ZendStackMode mode;
  void* value;
};

class ZendExecutionStack : public HPHP::RequestEventHandler {
  public:
    static zval* getArg(int i);

    static void push(void* z);
    static void* pop();
    static void pushHHVMStack();
    static void popHHVMStack();

  // Instance methods
  public:
    virtual void requestInit() {
      clear();
    }
    virtual void requestShutdown() {
      clear();
    }

  private:
    static std::vector<ZendStackEntry>& getStack();
    void clear() {
      m_stack.clear();
    }
    std::vector<ZendStackEntry> m_stack;
};

#endif // incl_HPHP_ZEND_EXECUTION_STACK
