<?hh

abstract class Foo {
  public Foo $prop = "aaa";
  public static Foo $static_prop = "aaa";
  const Foo CONST = "aaa";
  abstract const Foo ABS_CONST;
  const type type_const = Foo;

  public function method(Generic<Foo> $x, $y) {
    if ($y instanceof Foo) {
    }
    try {
    } catch (Foo $foo) {
    }
  }
}

type Alias = Foo;

trait T {
  require extends Foo; // TODO: this is not found
}

abstract class C extends Foo {}

function test<T as Foo>(
  Foo $c
): Foo {
  return new Foo();
}
