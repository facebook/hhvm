<?hh
class A {

  private static $testNotMemoizedI = 100;
  public static function testNotMemoized() :mixed{ return self::$testNotMemoizedI++; }

  private static $testStaticI = 110;
  <<__Memoize>>
  public static function testStatic() :mixed{ return self::$testStaticI++; }
}


<<__EntryPoint>>
function main_static() :mixed{
echo A::testNotMemoized().' ';
echo A::testNotMemoized().' ';
echo A::testStatic().' ';
echo A::testStatic()."\n";
}
