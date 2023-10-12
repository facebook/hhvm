<?hh

function f(int $x, inout mixed ...$ys): void {} // ERROR

function g1(inout string $s = 'g1'): void {} // ERROR

function g2(string $s = 'g2', inout int $i): void {} // ERROR

function g3(inout int $i, string $s = 'g3'): void {} // no error

function q(inout $x): void {} // ERROR

function v((function(inout mixed ...): void) $f): void {} // ERROR

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

  $c = async function(inout $v) {}; // ERROR
  $c = async (inout $v) ==> 42; // ERROR
}

async function gen_test(): void {
  $c = function(inout $f) {}; // no error
  $c = (inout $v) ==> 42; // no error
}
