<?hh // strict

type FooShape = shape(
  'foo' => int,
  'bar' => ?string,
  A::BAZ_KEY => array<string>,
);

class A {
  const string BAZ_KEY = 'baz';
  const FooShape
    FOO = shape(
      'foo' => 2,
      'bar' => null,
      self::BAZ_KEY => array('a', 'b', 'c'),
    );

  public static function getBaz(): array<string> {
    // Should throw because the shape uses a constant
    return self::FOO['baz'];
  }
}
