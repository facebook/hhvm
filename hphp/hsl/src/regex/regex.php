<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Regex;

use namespace HH\Lib\{_Private, Str};

/**
 * Returns the first match found in `$haystack` given the regex pattern `$pattern`
 * and an optional offset at which to start the search.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 *
 * Throws Invariant[Violation]Exception if `$offset` is not within plus/minus the length of `$haystack`.
 * Returns null if there is no match, or a Match containing
 *    - the entire matching string, at key 0,
 *    - the results of unnamed capture groups, at integer keys corresponding to
 *        the groups' occurrence within the pattern, and
 *    - the results of named capture groups, at string keys matching their respective names.
 */
function first_match<T as Match>(
  string $haystack,
  Pattern<T> $pattern,
  int $offset = 0,
)[]: ?T {
  return _Private\regex_match($haystack, $pattern, inout $offset);
}

/**
 * Returns all matches found in `$haystack` given the regex pattern `$pattern`
 * and an optional offset at which to start the search.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 *
 * Throws Invariant[Violation]Exception if `$offset` is not within plus/minus the length of `$haystack`.
 */
function every_match<T as Match>(
  string $haystack,
  Pattern<T> $pattern,
  int $offset = 0,
)[]: vec<T> {
  $haystack_length = Str\length($haystack);
  $result = vec[];
  while (true) {
    $match = _Private\regex_match($haystack, $pattern, inout $offset);
    if ($match === null) {
      break;
    }
    $result[] = $match;
    $match_length = Str\length(Shapes::at($match, 0) as string);
    if ($match_length === 0) {
      $offset++;
      if ($offset > $haystack_length) {
        break;
      }
    } else {
      $offset += $match_length;
    }
  }
  return $result;
}

/**
 * Returns whether a match exists in `$haystack` given the regex pattern `$pattern`
 * and an optional offset at which to start the search.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 *
 * Throws Invariant[Violation]Exception if `$offset` is not within plus/minus the length of `$haystack`.
 */
function matches(
  string $haystack,
  Pattern<Match> $pattern,
  int $offset = 0,
)[]: bool {
  return _Private\regex_match($haystack, $pattern, inout $offset) !== null;
}

/**
 * Returns `$haystack` with any substring matching `$pattern`
 * replaced by `$replacement`. If `$offset` is given, replacements are made
 * only starting from `$offset`.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 *
 * Throws Invariant[Violation]Exception if `$offset` is not within plus/minus the length of `$haystack`.
 */
function replace(
  string $haystack,
  Pattern<Match> $pattern,
  string $replacement,
  int $offset = 0,
)[]: string {
  // replace is the only one of these functions that calls into a native
  // helper other than match. It needs its own helper to be able to handle
  // backreferencing in the `$replacement` string. Our offset handling is
  // trivial so we do it here rather than pushing it down into the helper.
  $offset = _Private\validate_offset($offset, Str\length($haystack));

  if ($offset === 0) {
    list ($result, $error) =
      _Private\_Regex\replace($haystack, $pattern, $replacement);
    if ($error is nonnull) {
      throw new namespace\Exception($pattern, $error);
    }
    return $result as nonnull;
  }

  $haystack1 = Str\slice($haystack, 0, $offset);
  $haystack2 = Str\slice($haystack, $offset);
  list ($result, $error) =
    _Private\_Regex\replace($haystack2, $pattern, $replacement);
  if ($error is nonnull) {
    throw new namespace\Exception($pattern, $error);
  }
  return $haystack1 . ($result as nonnull);
}

/**
 * Returns `$haystack` with any substring matching `$pattern`
 * replaced by the result of `$replace_func` applied to that match.
 * If `$offset` is given, replacements are made only starting from `$offset`.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 *
 * Throws Invariant[Violation]Exception if `$offset` is not within plus/minus the length of `$haystack`.
 */
function replace_with<T as Match>(
  string $haystack,
  Pattern<T> $pattern,
  (function(T)[_]: string) $replace_func,
  int $offset = 0,
)[ctx $replace_func]: string {
  $haystack_length = Str\length($haystack);
  $result = Str\slice($haystack, 0, 0);
  $match_end = 0;
  while (true) {
    $match = _Private\regex_match($haystack, $pattern, inout $offset);
    if ($match === null) {
      break;
    }
    // Copy anything between the previous match and this one
    $result .= Str\slice($haystack, $match_end, $offset - $match_end);
    $result .= $replace_func($match);
    $match_length = Str\length(Shapes::at($match, 0) as string);
    $match_end = $offset + $match_length;
    if ($match_length === 0) {
      // To get the next match (and avoid looping forever), need to skip forward
      // before searching again
      // Note that `$offset` is for searching and `$match_end` is for copying
      $offset++;
      if ($offset > $haystack_length) {
        break;
      }
    } else {
      $offset = $match_end;
    }
  }
  $result .= Str\slice($haystack, $match_end);
  return $result;
}

/**
 * Splits `$haystack` into chunks by its substrings that match with `$pattern`.
 * If `$limit` is given, the returned vec will have at most that many elements.
 * The last element of the vec will be whatever is left of the haystack string
 * after the appropriate number of splits.
 * If no substrings of `$haystack` match `$delimiter`, a vec containing only `$haystack` will be returned.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 *
 * Throws Invariant[Violation]Exception if `$limit` < 2.
 */
function split(
  string $haystack,
  Pattern<Match> $delimiter,
  ?int $limit = null,
)[]: vec<string> {
  if ($limit === null) {
    $limit = \INF;
  }
  invariant(
    $limit > 1,
    'Expected limit greater than 1, got %d.',
    $limit,
  );
  $haystack_length = Str\length($haystack);
  $result = vec[];
  $offset = 0;
  $match_end = 0;
  $count = 1;
  $match = _Private\regex_match($haystack, $delimiter, inout $offset);
  while ($match && $count < $limit) {
    // Copy anything between the previous match and this one
    $result[] = Str\slice($haystack, $match_end, $offset - $match_end);
    $match_length = Str\length(Shapes::at($match, 0) as string);
    $match_end = $offset + $match_length;
    if ($match_length === 0) {
      // To get the next match (and avoid looping forever), need to skip forward
      // before searching again
      // Note that `$offset` is for searching and `$match_end` is for copying
      $offset++;
      if ($offset > $haystack_length) {
        break;
      }
    } else {
      $offset = $match_end;
    }
    $count++;
    $match = _Private\regex_match($haystack, $delimiter, inout $offset);
  }
  $result[] = Str\slice($haystack, $match_end);
  return $result;
}

/**
 * Renders a Regex Pattern to a string.
 * The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.
 */
function to_string(Pattern<Match> $pattern)[]: string {
  return $pattern as string;
}
