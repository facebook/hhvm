<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

abstract final class ThriftUniversalName {
  const THRIFT_SCHEME = "fbthrift://";
  const MIN_HASH_LENGTH = 8;
  const DEFAULT_HASH_LENGTH = 16;

  public static function matchesUniversalHash(
    string $universal_hash,
    string $prefix,
  )[]: bool {
    if (
      (Str\length($universal_hash) < Str\length($prefix)) ||
      Str\is_empty($prefix)
    ) {
      return false;
    }

    return Str\starts_with($universal_hash, $prefix);
  }

  <<__Memoize>>
  public static function getUriHash(string $uri)[]: string {
    return non_crypto_sha256(self::THRIFT_SCHEME.$uri, true);
  }

  public static function getHackTypeForHashPrefix<T>(
    vec<(string, classname<T>)> $entries,
    string $hash_prefix,
  )[]: ?classname<T> {
    $match = self::getMatchingEntryForHashPrefix($entries, $hash_prefix);
    if ($match is nonnull) {
      return $match[1];
    }

    return null;
  }

  private static function getMatchingEntryForHashPrefix<T>(
    vec<(string, classname<T>)> $entries,
    string $hash_prefix,
  )[]: ?(string, classname<T>) {
    $start_idx = C\fb\binary_search(
      $entries,
      $entry ==> Str\compare($entry[0], $hash_prefix) >= 0,
    );

    $predicate = (int $idx): bool ==> {
      return ($idx < C\count($entries)) &&
        self::matchesUniversalHash($entries[$idx][0], $hash_prefix);
    };

    if (!$predicate($start_idx)) {
      return null;
    }

    invariant(
      !$predicate($start_idx + 1),
      'Multiple types matched hash prefix, lookup is ambiguous.',
    );

    return $entries[$start_idx];
  }
}
