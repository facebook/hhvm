<?hh

class TestBase {
  public static function foo<reify T1, reify T2>() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
  }
}

final class Test extends TestBase {
  public static function bar(): void {
    $x = self::foo<int, string>;
    $x();
  }
}

<<__EntryPoint>> function main(): void {
  Test::bar();
}
