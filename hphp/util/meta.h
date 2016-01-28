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
#ifndef META_H_
#define META_H_

#include <type_traits>

namespace HPHP {

// is_const_iterator: by analogy with is_const et al.
template<typename Iter>
struct is_const_iterator {
  using pointer = typename std::iterator_traits<Iter>::pointer;
  using type = typename std::is_const<
    typename std::remove_pointer<pointer>::type
  >::type;

  static bool constexpr value = type::value;
};

// match_iterator: return Value with the same const-ness as Iter.
template<typename Iter,
         typename Value = typename std::iterator_traits<Iter>::type>
struct match_iterator : std::conditional<
  is_const_iterator<Iter>::value,
  std::add_const<Value>,
  std::remove_const<Value>
  >::type {};

// hphp_field_type: provide the type of a struct field.
#define hphp_field_type(strct, fld) decltype(((strct*)0)->fld)

}

#endif /* META_H_ */
