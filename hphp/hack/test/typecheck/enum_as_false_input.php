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

enum DerpyEnum: string as string {
  test1 = "haha";
  test2 = "DERP";
}

enum BaitEnum: string as string {
  test1 = "haha";
  test2 = "DERP";
}

function foo(): classname<DerpyEnum> {
  return BaitEnum::class;
}
