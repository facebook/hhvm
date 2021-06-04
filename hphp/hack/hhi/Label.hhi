<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of HHVM's builtin classes.
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH {
// TODO: mabe make more namespaces, like HH\Enum\Class

/**
 * Type of atoms
 */
newtype Label<-TEnumClass, TType> = string;
}
