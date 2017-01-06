/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_UTIL_ALGORITHM_H_
#define incl_HPHP_UTIL_ALGORITHM_H_

#include <algorithm>
#include <iterator>
#include <vector>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
/*
 * Like <algorithm>, but jankier.
 */

///////////////////////////////////////////////////////////////////////////////

/*
 * Sort the keys of a container by its values.
 */
template<class T, class Compare>
std::vector<size_t>
sort_keys_by_value(const std::vector<T>& input, Compare compare) {
  auto result = std::vector<size_t>{};
  result.reserve(input.size());
  for (auto i = 0; i < input.size(); ++i) result.emplace_back(i);

  std::sort(
    std::begin(result), std::end(result),
    [&] (size_t a, size_t b) { return compare(input[a], input[b]); }
  );
  return result;
}

template<class Container, class Compare>
std::vector<typename Container::key_type>
sort_keys_by_value(const Container& input, Compare compare) {
  using key_type = typename Container::key_type;

  auto result = std::vector<key_type>{};
  result.reserve(input.size());
  for (auto const& kv : input) result.emplace_back(kv.first);

  std::sort(
    std::begin(result), std::end(result),
    [&] (const key_type& a, const key_type& b) {
      return compare(input.at(a), input.at(b));
    }
  );
  return result;
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
