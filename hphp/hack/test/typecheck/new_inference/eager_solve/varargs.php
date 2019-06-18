<?hh // strict

/* HH_FIXME[4110] */
function concat(string $first, string ...$rest): string {
}

class Inv<T> {
  public function __construct(public T $value) {}
}

function test(): string {
  $ss = (new Inv(tuple('foo', 'bar')))->value;
  return concat(...$ss);
}
