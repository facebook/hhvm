<?hh // strict

class A {
  const A = 5;
}

class Nus {
  public function __construct(public int $baz) {}
  public function lol(this $x): this { return $this; }
}

function foo(Nus $a, ?Nus $b, arraykey $c, ?int $d): void {}

function bar<T>(T $a, Nus $b, mixed $c, resource $d, string $e,
                float $f, num $g): void {}

function shp(shape('a' => int, 'b' => string) $x): void {}
function shp2(shape(A::A => int) $x): void {}
function arr(array<string, int> $x): void {}
function noret(): noreturn { while (true); }

function tup((int, int) $x): (int, int) { return $x; }

function test(): void {

}
