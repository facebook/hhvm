<?hh

class C<reify T> {}

final class Test {
  public static function f<reify T>(T $x): C<T> {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
    return new C<int>();
  }
}

function wrap($fun) {
  try {
    $fun();
  } catch (Throwable $e) {
    print $e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $f = Test::f<int>;

  wrap(() ==> { $f("Hello World"); }); // Parameter type hint violation
}
