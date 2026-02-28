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
    HH\dynamic_class_meth(self::class, $arr[0])();
    HH\dynamic_class_meth(parent::class, $arr[0])();
    echo self::MYCONST . "\n";
    echo parent::MYCONST . "\n";
  }
}
<<__EntryPoint>> function main(): void {
C::test();
}
