<?hh

function foo(inout bool $io): void {
  $io = true;
}

function bar(string $x, inout int $y): void {
  $y = 42;
}

class C {
  public function baz(inout string $s): string {
    $s .= 'baz';
    return $s;
  }
}
