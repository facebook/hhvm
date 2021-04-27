<?hh /* -*- php -*- */
<<file:__EnableUnstableFeatures('readonly')>>
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

  function global_get(string $key)[globals]: mixed;
  function global_readonly_get(string $key)[read_globals]: readonly mixed;
  function global_isset(string $key)[read_globals]: bool;
  function global_key_exists(string $key)[read_globals]: bool;
  function global_set(string $key, mixed $value)[globals]: void;
  function global_unset(string $key)[globals]: void;

}
