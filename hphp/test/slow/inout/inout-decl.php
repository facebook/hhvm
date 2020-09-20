<?hh

namespace Q {
class R {
  function R(inout $S) {} // not a constructor because of the namespace
}
}

namespace {

const inout = 12;

class Cls {
  const int inout = 12;

  function __construct() {}
  function Cls(inout $foo) {} // not a constructor because of __construct
}

class Herp {
  function inout(inout $x) {}
}

class Derp {
  const int foo = Cls::inout;
}

trait T {
  function T(inout $x) {}
}

class C {
  use T;
}

class PrivGGParent {
  private function foo(inout $x) {}
  private function bar() {}
}

trait PrivTrait {
  private function foo($x) {}
  private function bar(inout $y) {}
}

class PrivGParent extends PrivGGParent {
  use PrivTrait;
}

class PrivParent extends PrivGParent {
  private function foo(inout $x) {}
  private function bar($y) {}
}

class PrivChild extends PrivParent {
  private function foo() {}
  private function bar(inout $y) {}
}

function foo($x, $y, inout $z, $q) {}
function bar(inout int $x) {}
function f<T>(inout vec<T> $v, ...$_) {}
function g($q, inout dict<string,vec<int>> $r, ...$_) {}
function h(inout $a, inout $b, $t, inout bool $c, $a = 12) {}

function fptr<T as (function(inout int, inout bool, inout float): arraykey)>(
  inout $a,
  (function(inout int, inout Foo, inout float): Bar) $b
): (function(inout float, inout int, float): int) {
}

function main($a, $b, inout $c, $d, $e) {
  if ($c === 3) return;

  foo(1, 2, inout $a, 3);
  bar(inout $a['x']);
  f(inout $a['v']['s'], $b['x'], $c);
  g($a, inout $b, $c);
  h(inout $a['m'], inout $b, $c, inout $a[$e], $e);

  $x = Cls::inout;
  $y = Herp::inout(inout $x);
  $z = Derp::foo + $x + $y;

  return $x + $y + $z;
}
<<__EntryPoint>> function main_entry(): void {
$a = 3;
main(1, 2, inout $a, 4, 5, 6);
\var_dump(Cls::inout, Derp::foo);
echo "Done.\n";
}
}
