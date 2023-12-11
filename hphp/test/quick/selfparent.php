<?hh
class B {
  const MYCONST = "B::MYCONST";
  public static function foo() :mixed{
    echo "B::foo\n";
  }
}
class C extends B {
  const MYCONST = "C::MYCONST";
  public static function foo() :mixed{
    echo "C::foo\n";
  }
  public static function test() :mixed{
    $arr = vec['foo'];
    self::foo();
    parent::foo();
    self::$arr[0]();
    parent::$arr[0]();
    echo self::MYCONST . "\n";
    echo parent::MYCONST . "\n";
  }
}
<<__EntryPoint>> function main(): void {
C::test();
}
