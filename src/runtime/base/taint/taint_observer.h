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

#ifndef __HPHP_TAINT_OBSERVER_H__
#define __HPHP_TAINT_OBSERVER_H__

#ifdef TAINTED

/**
 * The purpose of this class is to keep track of which strings are getting
 * "read" and which strings are getting created. This allows us to
 * taint strings by modifying the idl, and saves us from having taint related
 * code spread all over the hphp codebase.
 *
 * This code is meant to be created in the stack. You can therefore not
 * try to do things like new TaintObserver().
 */

#include <runtime/base/taint/taint_data.h>
#include <util/thread_local.h>

namespace HPHP {

class StringData;
class StringBuffer;

class TaintObserver {
public:
  /**
   * set_mask controls which bits get set for all new strings
   * clear_mask controls which bits will be cleared for all new strings
   * if set_mask and clear_mask are both set to 0, then we propagate
   * using the input strings.
   */
  TaintObserver(bitstring set_mask = 0, bitstring clear_mask = 0);
  ~TaintObserver();

  /**
   * This functions needs to be called whenever data inside strings is accessed.
   */
  static void RegisterAccessed(const StringData *string_data);
  static void RegisterAccessed(const StringBuffer *string_buffer);

  /**
   * This function needs to be called whenever a string is created or mutated.
   */
  static void RegisterMutated(StringData *string_data);
  static void RegisterMutated(StringBuffer *string_buffer);

private:
  bitstring m_set_mask;
  bitstring m_clear_mask;
  TaintObserver* m_previous;

  static DECLARE_THREAD_LOCAL(TaintObserver*, instance);

  // Disallow new, copy constructor and assignment operator
  void* operator new(long unsigned int size);
  TaintObserver(TaintObserver const&);
  TaintObserver& operator=(TaintObserver const&);

  TaintData m_current_taint;
};

}

// Helper macro. If you use TAINT_OBSERVER, you don't need to
// wrap things in #ifdef TAINTED ... #endif
#define TAINT_OBSERVER(set, clear) \
  TaintObserver taint_observer((set), (clear))

#define TAINT_OBSERVER_REGISTER_ACCESSED(obj) \
  TaintObserver::RegisterAccessed((obj))

#define TAINT_OBSERVER_REGISTER_MUTATED(obj) \
  TaintObserver::RegisterMutated((obj))

#else

#define TAINT_OBSERVER(set, clear) /* do nothing (note: not ; friendly) */
#define TAINT_OBSERVER_REGISTER_ACCESSED(obj) /* do nothing */
#define TAINT_OBSERVER_REGISTER_MUTATED(obj) /* do nothing */

#endif // TAINTED

#endif // __HPHP_TAINT_OBSERVER_H__
