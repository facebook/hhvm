<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type my_t = int;

function f1($p) { return $p; }
function f2($p): int { return $p; }
function f3($p): string { return $p; }
function f4($p): bool { return $p; }
function f5($p): float { return $p; }
function f6($p): resource { return $p; }
function f7($p): array { return $p; }
function f8($p): callable { return $p; }
function f9($p): Shapes { return $p; }
function f10($p): Square { return $p; }
function f11($p): :div { return $p; }
function f12($p): Fractal<Square> { return $p; }
function f13<T>($p): Fractal<T> { return $p; }
function f14($p): my_t { return $p; }
function f15($p): void { return $p; }
function f16($p): mixed { return $p; }
function f17($p): ?int { return $p; }
function f18($p): (string, int) { return $p; }
function f19($p): (function(int): int) { return $p; }
function f20($p): callable { return $p; }

class Shapes {}
class Square extends Shapes {}
class Fractal<T> extends Shapes {}
class :div {}

class A {}
class B extends A {
  public function f21(): this { return $this; }
  public function f22($p): this { return $p; }
  public static function f23($p): self { return $p; }
  public static function f24($p): parent { return $p; }
  public static function testfunc() {}
}
class C extends B {}

function testfunc() {}

function main() {
  for ($i = 1; $i <= 20; $i++) {
    $f = 'f' . $i;
    echo "\ncalling $f\n";
    $f(null);
    $f(42);
    $f('foobar');
    $f(true);
    $f(14.1);
    $f(STDIN);
    $f(array());
    $f(function($x){return $x*$x;});
    $f(new Shapes());
    $f(new Square());
    $f(new Fractal());
    $f(<div/>);
    $f('testfunc');
    $f(array('C', 'testfunc'));
  }

  $c = new C();
  echo "\ncalling f21\n";
  $c->f21();

  $callbacks = Map {
    'f22' => array($c, 'f22'),
    'f23' => array('C', 'f23'),
    'f24' => array('C', 'f24')
  };
  foreach ($callbacks as $name => $f) {
    echo "\ncalling $name\n";
    $f(null);
    $f(42);
    $f('foobar');
    $f(true);
    $f(14.1);
    $f(STDIN);
    $f(array());
    $f(function($x){return $x*$x;});
    $f(new Shapes());
    $f(new Square());
    $f(new Fractal());
    $f(<div/>);
    $f(new A());
    $f(new B());
    $f(new C());
  }
}
main();
