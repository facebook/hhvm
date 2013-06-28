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

#ifndef incl_HPHP_BITS_H_
#define incl_HPHP_BITS_H_

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct BitInfo {
  static const T NumBits  = sizeof(T) * 8;
  static const T HighMask = 0x1 << (NumBits - 1);
};

template <unsigned int i, unsigned int m>
struct BitCountImpl {
  enum { value = ((i & m) ? 1 : 0) + BitCountImpl< i, (m >> 1) >::value };
};

template <unsigned int i>
struct BitCountImpl<i, 0> {
  enum { value = 0 };
};

template <unsigned int i>
struct BitCount: public BitCountImpl<i,
                                     BitInfo<unsigned int>::HighMask>
{};

template <unsigned int i, bool p, unsigned int m>
struct BitPhaseImpl {
  enum {
    value = ((i & m) ? (p ? 0 : 1) : (p ? 1 : 0)) +
            BitPhaseImpl<i, ((i & m) != 0), (m >> 1)>::value
  };
};

template <unsigned int i, bool p>
struct BitPhaseImpl<i, p, 0> {
  enum { value = 0 };
};

template <unsigned int i>
struct BitPhase : public BitPhaseImpl<i,
                                      i & BitInfo<unsigned int>::HighMask,
                                      BitInfo<unsigned int>::HighMask>
{};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BITS_H_
