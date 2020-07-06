<?hh

final class Test {
  public static function foo<reify T>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main(): void {
  try {
    $f = Test::foo<_>;
    $f();
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
