<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace Foo {
  trait TN {
    <<__Memoize>>public function test() {static $i = 1000; return $i++;}
  }
}
