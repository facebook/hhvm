<?hh
class A {

  private static $testNotMemoizedI = 100;
  public static function testNotMemoized() { return self::$testNotMemoizedI++; }

  private static $testStaticI = 110;
  <<__Memoize>>
  public static function testStatic() { return self::$testStaticI++; }
}


<<__EntryPoint>>
function main_static() {
echo A::testNotMemoized().' ';
echo A::testNotMemoized().' ';
echo A::testStatic().' ';
echo A::testStatic()."\n";
}
