<?hh


namespace Foo {
  trait TN {
    public static $i = 1000;
    <<__Memoize>>public function test() {return self::$i++;}
  }
}
