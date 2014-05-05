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


class B {
  public function doTheFoo(): void {
    print "thefoo";
  }
}

class A {


  private function useB(): void {
    $b = $this->getMyB();
    if (!$b instanceof B) {
      // nothing here;
    } else {
      $b->doTheFoo();
    }
  }

  private function getMyB(): ?B {
    return new B();
  }

}

