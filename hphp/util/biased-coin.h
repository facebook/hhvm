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

#ifndef incl_HPHP_BIASED_COIN_H_
#define incl_HPHP_BIASED_COIN_H_

#include "hphp/util/assertions.h"
#include "hphp/util/trace.h"

namespace HPHP {

class BiasedCoin {
  int m_seed;
  double m_pct;
  unsigned m_state;

 public:
  explicit BiasedCoin(double pct, unsigned seed = -1u)
    : m_seed(seed)
    , m_pct(pct)
  {
    assert(pct <= 100.0);
    if (seed == -1u) {
      seed = getpid();
    }
    reset();
  }

  BiasedCoin(Trace::Module pct, Trace::Module seed) {
    using namespace HPHP::Trace;
    // default to flip() always returning false
    m_pct  = moduleEnabled(pct) ? double(moduleLevel(pct)) : 0.0;
    m_seed = moduleEnabled(seed) ? moduleLevel(seed) : getpid();
    reset();
  }

  double getPercent() const { return m_pct; }

  int getSeed() const { return m_seed; }

  void reset() {
    m_state = m_seed;
  }

  bool flip() {
    int val = rand_r(&m_state);
    return 100.0 * (val / (RAND_MAX + 1.0)) < m_pct;
  }
};

}

#endif
