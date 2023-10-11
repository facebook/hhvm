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

enum SomeEnum: string as string {
  test1 = "testString1";
  test2 = "testString2";
}

function foo(): classname<SomeEnum> {
  return SomeEnum::class;
}
