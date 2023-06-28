<?hh

interface I {}

abstract class X {}

abstract class Y extends X implements I {
}

class Z extends Y {
  function foo($x) : int { return (int)$x; }
}

function x(X $x) : X {
  if ($x is Y) return $x;
  return new Z;
}

function g(X $x, int $i) :mixed{
  $x = x($x);
  if ($x is I) {
    return $x->foo($i);
  }
  return ~42;
}
<<__EntryPoint>> function main(): void {
echo g(new Z, 42), "\n";
}
