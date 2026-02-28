<?hh

interface I {}
class A implements I {}
class B implements I {}

function test(bool $b): I {
  list($x1, $x2) = $b ? Pair { 1, new A() } : Pair { 2, new B() };
  return $x2;
}
