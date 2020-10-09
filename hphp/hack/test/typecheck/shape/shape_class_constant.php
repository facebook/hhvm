<?hh // strict

type FooShape = shape(
  'foo' => int,
  'bar' => ?string,
  A::BAZ_KEY => varray<string>,
);

class A {
  const string BAZ_KEY = 'baz';
  const FooShape
    FOO = shape(
      self::BAZ_KEY => varray['a', 'b', 'c'],
      'foo' => 2,
      'bar' => null,
    );

  public static function getBaz(): varray<string> {
    // Should throw because the shape uses a constant
    return self::FOO['baz'];
  }
}
