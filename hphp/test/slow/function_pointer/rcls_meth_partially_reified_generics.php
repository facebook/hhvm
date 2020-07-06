<?hh

final class Test {
  public static function foo<T, reify Tb>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<Tb>());
  }
}

<<__EntryPoint>>
function main(): void {
  try {
    $f = Test::foo<_, int>;
    $f();
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
