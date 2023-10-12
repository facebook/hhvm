<?hh // strict
abstract class Foo {
  abstract const type TFoo as shape(
    ?'foo' => int,
    ...
  );
  public static abstract function getFoo(): this::TFoo;
}

class Bar extends Foo {
  const type TFoo = shape(
    ?'foo' => int,
    ...
  );
  public static function getFoo(): this::TFoo {
    return shape('foo' => 1);
  }
}

class Baz {
  const vec<classname<Foo>> FOOS = vec[Bar::class];
  public static function bazzy(): void {
    $class = self::FOOS[0];
    $test = $class::getFoo();
    $foo = $test['foo'] ?? null;
  }
}
