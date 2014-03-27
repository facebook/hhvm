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

// Another HPHP extension, implicit fields.

// Scala has also this feature, see point 2 in
// http://codemonkeyism.com/top-5-things-to-know-about-constructors-in-scala/
// See also section 5.3 of the scala reference manual.

class A {
  public $y;
  // this is desugar as
  // public $x; __construct($x) { $this->x = $x; }
  public function __construct(public $x, $y) {
    $this->y = $y;
  }
  
}
