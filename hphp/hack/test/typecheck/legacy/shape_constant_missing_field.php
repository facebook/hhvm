<?hh // strict

type FooShape = shape(
  'foo' => int,
  'bar' => string,
);

class A {
  const FooShape FOO = shape('foo' => 2);
}
