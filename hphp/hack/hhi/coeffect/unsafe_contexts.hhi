<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

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
  // TODO(coeffects)
  // TODO(coeffects) after implementing lower bounds on const ctx/type, do:
  // = \HH\Capabilities\RxLocal

  type cipp_global = mixed;
  type cipp_of<T> = mixed;
  type cipp = mixed;

  type output = mixed;

  type local = mixed;

  type rx = mixed;
  type rx_shallow = \HH\Capabilities\RxLocal;
  type rx_local = \HH\Contexts\defaults;
}
