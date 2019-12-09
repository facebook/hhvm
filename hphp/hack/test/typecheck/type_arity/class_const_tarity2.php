<?hh

type Bar<T> = shape(
  'hello' => T,
);

final class Foo {
  const Bar<int> BAZ = shape('hello' => 1);
}
