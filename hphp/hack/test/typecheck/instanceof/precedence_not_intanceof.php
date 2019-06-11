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


class B {
  public function doTheFoo(): void {
    print "thefoo";
  }
}

class A {


  private function useB(): void {
    $b = $this->getMyB();
    if (!$b is B) {
      // nothing here;
    } else {
      $b->doTheFoo();
    }
  }

  private function getMyB(): ?B {
    return new B();
  }

}

