<?hh // strict

function f(int $x, inout mixed ...$ys): void {} // ERROR

function g1(inout string $s = 'g1'): void {} // ERROR

function g2(string $s = 'g2', inout int $i): void {} // ERROR

function g3(inout int $i, string $s = 'g3'): void {} // no error

function h(mixed $m, inout bool &$res): void {} // ERROR

function q(inout $x): void {} // ERROR

function z(inout vec<int> $v): void {
  $v[] = 42;
} // no error

class C {
  public static function x((function(): bool) $p, inout ?C $c): void {
    if ($p()) {
      $c = new self();
    }
  } // no error
}

function test(): void {
  $i = 999;
  g3(inout $i, 'ok'); // no error

  $v = vec[];
  z(inout &$v); // ERROR
}
