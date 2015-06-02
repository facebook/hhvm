<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

final class :ui:link {}

class Bloo {
  public function mybloo(): void {
    $x = <ui:link>abcd `</ui:link>;
  }
}
