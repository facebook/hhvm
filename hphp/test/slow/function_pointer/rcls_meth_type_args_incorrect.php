<?hh

class Test {
  public static function foo<reify T>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

function accepts_reified_rcls_meth(vec<mixed> $f): void {
  $f();
}

<<__EntryPoint>>
function main(): void {
  accepts_reified_rcls_meth(Test::foo<int>);
}
