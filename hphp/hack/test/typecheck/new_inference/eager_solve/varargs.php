<?hh // strict

function concat(string $first, string ...$rest): string {
  // UNSAFE
}

class Inv<T> {
  public function __construct(public T $value) {}
}

function test(): string {
  $ss = (new Inv(tuple('foo', 'bar')))->value;
  return concat(...$ss);
}
