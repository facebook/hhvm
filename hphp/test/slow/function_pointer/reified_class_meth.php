<?hh

final class Test {
  public static function foo<reify T1, reify T2>() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
  }
}


final class Test2 {
  public static function foo<reify T1 as Test>() :mixed{
    $x = T1::foo<int, string>;
    $x();
  }
}

<<__EntryPoint>> function main(): void {
  Test2::foo<Test>();
}
