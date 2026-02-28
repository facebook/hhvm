<?hh

final class Foo<T> {}

function get_like_foo(): Foo<mixed> {
  throw new Exception();
}

function test(): void {
  $f = get_like_foo();
  if ($f is null) {
    echo "never going to happen";
  }
}
