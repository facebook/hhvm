<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace Bar {
  trait TN {
    use \Foo\TN;
    <<__Memoize>>public function test($a = 0) {
      static $i = 30;
      return $a + $i++;
    }
  }
}
