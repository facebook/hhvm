<?hh

type Bar<T> = shape(
  'hello' => T,
);

final class Foo {
  const Bar BAZ = shape('hello' => 1);
}
