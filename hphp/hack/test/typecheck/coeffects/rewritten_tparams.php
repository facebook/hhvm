<?hh

final class P {
  const ctx C = [];
}

function f(
  (function ()[_]: void) $f,
  P $v
)[ctx $f, $v::C]: void {
  print "hi";
}

function g(P $v)[$v::C]: int {
  return $v;
}
