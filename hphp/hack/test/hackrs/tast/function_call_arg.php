<?hh

function foo(int $x, string $y): void {

}

function bar(int $x, string $y): void {
  foo($x, $y);
}
