<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

  function global_get(string $key): mixed;
  function global_isset(string $key): bool;
  function global_keys(): keyset<string>;
  function global_key_exists(string $key): bool;
  function global_set(string $key, mixed $value): void;
  function global_unset(string $key): void;

}
