<?hh

function foo(bool $bool, int $int): void {
  if ($bool) {
  }
  if ($int) {
  }
  while ($int) {
  }
  $int;
  do {
  } while ($int);
  for ($i = 0; $int; $i++) {
  }
  $bool && $int;
  $int && $bool;
  $bool || $int;
  $int || $bool;
  !$int;
  $int ? 1 : 2;
  $int ?: 2;
}

final class Box<T> {
  public function __construct(public T $value) {}
}

function bar(Box<int> $r): void {
  if ($r->value) {
  }
}
