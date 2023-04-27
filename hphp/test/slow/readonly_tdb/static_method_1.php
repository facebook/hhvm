<?hh
namespace ROTDB\StaticMethod1;
class Foo {
  public int $x = 0;
  public static int $sprop = 0;
  public static function class_method_mut(): Foo {
    return new Foo();
  }
  public static function class_method_ro(): readonly Foo {
    return new Foo();
  }
}

function ro(): readonly Foo {
  return new Foo();
}

function mut(): Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  $foo_mut = Foo::class_method_mut();
  $foo_mut->x = 3;

  $foo_ro = Foo::class_method_ro(); // inferred readonly
  $foo_ro->x = 3; // fatal
}
