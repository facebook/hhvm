<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

<<file: __EnableUnstableFeatures('class_level_where')>>

class C {}

interface I where this as C {}

trait PartialC implements I where this = C {}

interface J where C super this {}
