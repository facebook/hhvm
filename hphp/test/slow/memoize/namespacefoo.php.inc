<?hh


namespace Foo {
  trait TN {
    <<__Memoize>>public function test() {static $i = 1000; return $i++;}
  }
}
