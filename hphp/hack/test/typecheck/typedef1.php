<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class A {}

type MyTypedef = A;

/* HH_FIXME[4123]: cascading errors */
class X extends MyTypedef {
}
