<?hh
namespace ROTDB\StaticMethod2;
class Foo {
  public int $x = 0;
  public static function class_method_mut(): Foo {
    return new Foo();
  }
  public static function class_method_ro(): readonly Foo {
    return new Foo();
  }
}

<<__EntryPoint>>
function main(): void {
  $f = Foo::class_method_mut<>;
  $foo_mut = $f();
  $foo_mut->x = 3;

  $g = Foo::class_method_ro<>;
  $foo_ro = $g(); // inferred readonly
  $foo_ro->x = 3; // fatal
}
