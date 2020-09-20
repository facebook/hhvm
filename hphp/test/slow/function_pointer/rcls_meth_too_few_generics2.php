<?hh

final class Test {
  public static function foo<reify T, reify Tb>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main(): void {
  try {
    $f = Test::foo<int>;
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
