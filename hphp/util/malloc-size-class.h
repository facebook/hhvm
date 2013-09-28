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
#ifndef incl_HPHP_UTIL_MALLOC_SIZE_CLASS_H_
#define incl_HPHP_UTIL_MALLOC_SIZE_CLASS_H_

#include <boost/mpl/bool.hpp>
#include <boost/mpl/int.hpp>

#include "hphp/util/assertions.h"

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Define two metafunctions:
 *
 *   - is_malloc_size_class<Size>
 *
 *       Returns true is Size is a malloc size class, or true if we
 *       don't know anything about the malloc implementation and can't
 *       tell.
 *
 *   - next_malloc_size_class<Size>
 *
 *       Returns the next size class larger than or equal to Size, or
 *       returns Size if we don't know anything about this malloc.
 */

//////////////////////////////////////////////////////////////////////

#ifndef USE_JEMALLOC

template<size_t Size>
struct is_malloc_size_class
  : boost::mpl::true_
{};

template<size_t Size>
struct next_malloc_size_class
  : boost::mpl::int_<Size>
{};

#else

/*
 * jemalloc-specific implementation.
 */

template<size_t Size>
struct is_malloc_size_class
  : boost::mpl::bool_<
      // Small classes:
         Size == 8
     || (Size <=  128 && !(Size % 16))
     || (Size <=  256 && !(Size % 32))
     || (Size <=  512 && !(Size % 64))
     || (Size <= 1024 && !(Size % 128))
     || (Size <= 2048 && !(Size % 256))
     || (Size <= 4096 && !(Size % 512))
     // Large:
     || (Size <= 4096 * 1024 && !(Size % 4096))
     // Huge:
     || !(Size % (4096 * 1024))
    >
{};

template<size_t Size>
class next_malloc_size_class {
  template<size_t Multiple>
  struct round {
    static const size_t value =
      !(Size % Multiple) ? Size : Size + Multiple - Size % Multiple;
  };

public:
  typedef typename boost::mpl::int_<
    // Small classes:
    Size <= 8    ? 8 :
    Size <= 16   ? 16 :
    Size <= 128  ? round< 16>::value :
    Size <= 256  ? round< 32>::value :
    Size <= 512  ? round< 64>::value :
    Size <= 1024 ? round<128>::value :
    Size <= 2048 ? round<256>::value :
    Size <= 4096 ? round<512>::value :
    // Large:
    Size <= 4096 * 1024 ? round<4096>::value :
    // Huge:
    round<4096 * 1024>::value
  >::type type;
  static const size_t value = type::value;

  static_assert(is_malloc_size_class<value>::value,
                "Bug in malloc-size-class.h");
};

#endif

//////////////////////////////////////////////////////////////////////

}

#endif
