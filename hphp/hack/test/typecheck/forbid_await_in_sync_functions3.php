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

function sjm() {
  return function(Awaitable<mixed> $the_doctor) {
    await $the_doctor;
  };
}
