<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// see contexts.hhi for more details
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

/**
 * As an unsafe extension and for the purpose of top-level migration,
 * we additionally map certain contexts to a (set of) capabilities that
 * are provided but not required by a function (i.e., they are unsafe).
 * This namespace contains mapping to such capabilities using
 * same-named contexts. More precisely, the function/method with context
 * `ctx` has the following type of capability in its body:
 *   \HH\Capabilities\ctx & \HH\Capabilities\Unsafe\ctx
 * where safe contexts have `\Unsafe\ctx = mixed`. The function signature's
 * capability remains:
 *   \HH\Capabilities\ctx
 * for the purposes of subtyping and calling.
 */
namespace HH\Contexts\Unsafe {
  type defaults = mixed;

  type write_props = mixed;
  type read_globals = mixed;
  type globals = mixed;

  type leak_safe = mixed;
  type leak_safe_shallow = \HH\Capabilities\SystemLocal;
  type leak_safe_local = \HH\Contexts\defaults;

  type zoned = mixed;
  type zoned_shallow =
    (\HH\Capabilities\ImplicitPolicyLocal & \HH\Capabilities\SystemLocal &);
  type zoned_local = \HH\Contexts\defaults;
  type zoned_with<T> = mixed;

  type rx = \HH\Contexts\defaults;
  type rx_shallow = \HH\Contexts\defaults;
  type rx_local = \HH\Contexts\defaults;
}
