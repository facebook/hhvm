<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

enum DerpyEnum : string as string {
  test1 = "haha";
  test2 = "DERP";
}

enum BaitEnum : string as string {
  test1 = "haha";
  test2 = "DERP";
}

function foo(): classname<DerpyEnum> {
  return BaitEnum::class;
}
