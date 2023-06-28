<?hh

class TestBase {
  public static function foo<reify T1, reify T2>() :mixed{
    echo "TestBase::foo\n";
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
  }

  public static function bar() :mixed{
    echo "TestBase::bar\n";
    $x = static::foo<int, string>;
    $x();
  }
}

final class Test extends TestBase {
  <<__Override>>
  public static function foo<reify T1, reify T2>() :mixed{
    echo "Test::foo\n";
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
  }
}

<<__EntryPoint>>
function main(): void {
  TestBase::bar();
  Test::bar();
}
