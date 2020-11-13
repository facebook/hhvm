<?hh

class A { }
class B extends A { }

<<__InferFlows>>
function add((int|float) $x, int $y): (int|float) {
  return $x + $y;
}

<<__InferFlows>>
function qq(?B $x, A $y): A {
  return $x ?? $y;
}

<<__InferFlows>>
function le_datetime(DateTime $x, DateTime $y): bool {
  return $x <= $y;
}

<<__InferFlows>>
function diff(B $x, A $y): bool {
  return $x != $y;
}

<<__InferFlows>>
function eqeqeq(B $x, A $y): bool {
  return $x === $y;
}

<<__InferFlows>>
function is_((A | int | shape('fld' => int)) $x): bool {
  return $x is B;
}
