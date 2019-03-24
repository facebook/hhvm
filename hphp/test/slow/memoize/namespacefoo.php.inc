<?hh


namespace Foo {
  trait TN {
    static $i = 1000;
    <<__Memoize>>public function test() {return self::$i++;}
  }
}
