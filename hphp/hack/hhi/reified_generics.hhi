<?hh   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\ReifiedGenerics;

/**
 * Returns the type structure representation of the reified type
 */
<<__Rx>>
function get_type_structure<reify T>(): TypeStructure<T>;

/**
 * Returns the name of the class represented by this reified type.
 * If this type does not represent a class, throws an exception
 */
<<__Rx>>
function get_classname<reify T>(): classname<T>;
