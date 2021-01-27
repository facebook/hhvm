<?hh

function f(
  (function ()[_]: void) $f,
  vec<int> $v
)[ctx $f, $v::C]: void {
  print "hi";
}

function g(vec<int> $v)[$v::C]: int {
  return $v;
}

function h(
  ?(function ()[_]: void) $f,
  ?vec<int> $v
)[ctx $f, $v::C]: void {
  print "hi";
}
