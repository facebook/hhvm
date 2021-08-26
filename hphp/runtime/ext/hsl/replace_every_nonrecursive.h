#pragma once
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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


namespace HPHP {

/* Slow but generic, prefer `HHVM_FN(strtr)` if possible.
 *
 * Can be used for any encoding/string type, and for both case-sensitive
 * and case-insensitive.
 */
template<class T> String replace_every_nonrecursive(
  const String& raw_haystack,
  const Array& raw_replacements,
  std::function<T(const String&)> to_t,
  std::function<String(const T&)> from_t,
  std::function<void(T*)> normalize,
  std::function<void(T*)> fold_case,
  std::function<int32_t(const T& haystack, const T& needle, int32_t offset)> find,
  std::function<void(T* str, int32_t offset, int32_t len, const T& replacement)> replace
) {
  auto haystack = to_t(raw_haystack);
  normalize(&haystack);
  auto search(haystack);
  fold_case(&search);

  std::vector<std::tuple<T, T>> replacements;
  std::set<T> seen;

  IterateKV(raw_replacements.get(), [&](TypedValue rawNeedle, TypedValue rawReplacement) {
    assertx(isStringType(rawNeedle.type()));
    assertx(isStringType(rawReplacement.type()));
    auto needle = to_t(String(rawNeedle.val().pstr));
    normalize(&needle);
    fold_case(&needle);
    auto replacement = to_t(String(rawReplacement.val().pstr));
    replacements.push_back(std::make_tuple(needle, replacement));
    auto [it, is_unique] = seen.emplace(needle);
    if (!is_unique) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Duplicate replacement found after normalization"
      );
    }
  });

  // Longest matching needle wins
  std::sort(replacements.begin(), replacements.end(), [](
    decltype(replacements[0])& a,
    decltype(replacements[0])& b) {
      return std::get<0>(a).length() > std::get<0>(b).length();
  });

  int32_t i = 0;
  while (i < haystack.length()) {
    int32_t matchedOffset = -1;
    T matchedNeedle, matchedReplacement;

    for (const auto& [needle, replacement]: replacements) {
#ifndef NDEBUG
      if (matchedOffset >= 0) {
        assertx(matchedNeedle.length() >= needle.length());
      }
#endif
      auto j = find(search, needle, i);
      if (j < 0 || (matchedOffset >= 0 && j >= matchedOffset)) {
        continue;
      }
      if (matchedOffset == j && needle.length() < matchedNeedle.length()) {
        break;
      }
      matchedOffset = j;
      matchedNeedle = needle;
      matchedReplacement = replacement;
    }
    if (matchedOffset == -1) {
      break;
    }

    replace(&search, matchedOffset, matchedNeedle.length(), matchedReplacement);
    replace(&haystack, matchedOffset, matchedNeedle.length(), matchedReplacement);
    i = matchedOffset + matchedReplacement.length();
  }
  return from_t(haystack);
}

} // HPHP
