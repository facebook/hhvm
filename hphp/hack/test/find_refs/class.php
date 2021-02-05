<?hh

abstract class Foo {
  public Foo $prop = "aaa";
  public static Foo $static_prop = "aaa";
  const Foo CONST = "aaa";
  abstract const Foo ABS_CONST;
  const type type_const = Foo;

  public function method(Generic<Foo> $x, $y) {
    if ($y is Foo) {
    }
    try {
      might_throw();
    } catch (Foo $foo) {
    }
  }
  public static function staticMethod() {}
  public static function anotherStaticMethod() {
    self::staticMethod();
    self::$static_prop = "bbb";
  }
}

function might_throw(): void {}

type Alias = Foo;

trait T {
  require extends Foo;
}

abstract class C extends Foo {
  public static function yetAnotherStaticMethod() {
    parent::staticMethod();
  }
}

function test<T as Foo>(
  Foo $c
): Foo {
  return new Foo();
}

function special_funcs() {
  class_meth(Foo::class, 'staticMethod');
  meth_caller(Foo::class, 'method');

  class_meth('Foo', 'staticMethod');
  meth_caller('Foo', 'method');

  class_meth(Alias::class, 'staticMethod'); // TODO: This is not detected
  meth_caller(Alias::class, 'method'); // TODO: This is not detected

  class_meth('Alias', 'staticMethod'); // TODO: This is not detected
  meth_caller('Alias', 'method'); // TODO: This is not detected
}
