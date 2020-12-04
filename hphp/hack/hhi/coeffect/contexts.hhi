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
 * To concisely model multiple capabilities, we use intersection types
 * (denoted by &). This is an exact approximation, unlike introducing
 * a new subtype via interface hierarchies and using multiple inheritance.
 * (In case of the latter, introducing all parent capabilities separately
 * would not be equal to introducing them via the subtype capability.)
 */
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

namespace HH\Contexts {
  /**
   * This namespace provides a mapping between user-facing contexts
   * and a set of capabilities irrelevant to the user. The latter is
   * modeled as a series of interfaces, each with:
   * - a lowercase name (so that mapping is syntactical, avoiding extra logic)
   * - extends clause that lists all other capabilities that are subsumed,
   *   i.e., automatically present in the corresponding context
   */

  /**
   * The default, normally unannotated context. This is currently hardcoded in
   * Typing_make_type.default_capability for performance reasons. The alias is
   * still present so that it may be directly used as [defaults]
   */
  type defaults = (
    \HH\Capabilities\IO &
    \HH\Capabilities\AccessStaticVariable &
    \HH\Capabilities\WriteProperty
  );

  type cipp_global = (\HH\Capabilities\CippGlobal & \HH\Capabilities\IO & \HH\Capabilities\AccessStaticVariable);
  // type cipp<T> = (\HH\Capabilities\Cipp<T> & cipp_global);
  type cipp<T> = (\HH\Capabilities\Cipp<T> & \HH\Capabilities\IO & \HH\Capabilities\AccessStaticVariable);

  type non_det = \HH\Capabilities\NonDet;

  type rx = (\HH\Capabilities\Rx & \HH\Capabilities\IO);
  // type rx_shallow = (\HH\Capabilities\RxShallow & rx);
  type rx_shallow = (\HH\Capabilities\RxShallow & \HH\Capabilities\IO);
  // type rx_local = (\HH\Capabilities\RxLocal & rx_shallow);
  type rx_local = (\HH\Capabilities\RxLocal & \HH\Capabilities\IO);
}
