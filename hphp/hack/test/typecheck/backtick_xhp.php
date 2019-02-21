<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

final class :ui:link {}

class Bloo {
  public function mybloo(): void {
    $x = <ui:link>abcd `</ui:link>;
  }
}
