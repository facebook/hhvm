<?hh
class A {

  private static $testNotMemoizedI = 100;
  public static function testNotMemoized() :mixed{
    $__lval_tmp_0 = self::$testNotMemoizedI;
    self::$testNotMemoizedI++;
    return $__lval_tmp_0;
}

  private static $testStaticI = 110;
  <<__Memoize>>
  public static function testStatic() :mixed{
    $__lval_tmp_1 = self::$testStaticI;
    self::$testStaticI++;
    return $__lval_tmp_1;
}
}


<<__EntryPoint>>
function main_static() :mixed{
echo A::testNotMemoized().' ';
echo A::testNotMemoized().' ';
echo A::testStatic().' ';
echo A::testStatic()."\n";
}
