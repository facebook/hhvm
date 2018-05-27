<?hh // strict

final class User<T> {
  public function __construct(
    public T $id,
  ) {}
}

function foo(mixed $x): bool {
  return $x is vec<User<_>>;
}

function bar(mixed $x): bool {
  return $x is _;
}
