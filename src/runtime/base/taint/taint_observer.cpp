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

#ifdef TAINTED

#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/string_data.h>
#include <runtime/base/util/string_buffer.h>

namespace HPHP {

IMPLEMENT_THREAD_LOCAL(TaintObserver*, TaintObserver::instance);

TaintObserver::TaintObserver(bitstring set_mask, bitstring clear_mask) {
  m_set_mask = set_mask;
  m_clear_mask = clear_mask;

  m_previous = *instance;
  *instance = this;
}

TaintObserver::~TaintObserver() {
  *instance = m_previous;
}

void TaintObserver::RegisterAccessed(const StringData* string_data) {
  if (!*instance) {
    return;
  }

  // Prevent recursive calls into the TaintObserver. This should never
  // actually happen, except when adding debugging code.

  TaintObserver *tc = *instance;
  *instance = NULL;

  tc->m_current_taint.setTaint(
    string_data->getTaintDataRef().getTaint(),
    string_data->getTaintDataRef().getOriginalStr());

  *instance = tc;
}

void TaintObserver::RegisterAccessed(const StringBuffer *string_buffer) {
  if (!*instance) {
    return;
  }

  // Prevent recursive calls into the TaintObserver. This should never
  // actually happen, except when adding debugging code.

  TaintObserver *tc = *instance;
  *instance = NULL;

  tc->m_current_taint.setTaint(
    string_buffer->getTaintDataRef().getTaint(),
    string_buffer->getTaintDataRef().getOriginalStr());

  *instance = tc;
}

void TaintObserver::RegisterMutated(StringData* string_data) {
  if (!*instance) {
    return;
  }

  // Prevent recursive calls into the TaintObserver. This should never
  // actually happen, except when adding debugging code.

  TaintObserver *tc = *instance;
  *instance = NULL;

  bitstring t = tc->m_current_taint.getTaint();
  bitstring result_mask = tc->m_set_mask | (~tc->m_clear_mask & t);

  string_data->getTaintData()->setTaint(
    result_mask,
    tc->m_current_taint.getOriginalStr());

  *instance = tc;
}

void TaintObserver::RegisterMutated(StringBuffer* string_buffer) {
  if (!*instance) {
    return;
  }

  // Prevent recursive calls into the TaintObserver
  TaintObserver *tc = *instance;
  *instance = NULL;

  bitstring t = tc->m_current_taint.getTaint();
  bitstring result_mask = tc->m_set_mask | (~tc->m_clear_mask & t);

  string_buffer->getTaintData()->setTaint(
    result_mask,
    tc->m_current_taint.getOriginalStr());

  *instance = tc;
}

}

#endif // TAINTED
