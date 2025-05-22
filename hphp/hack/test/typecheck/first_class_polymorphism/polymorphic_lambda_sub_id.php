<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

function fork(
  (function<T>(T): T) $id,
  string $x,
  int $y
): (string,int) {
  $x = $id($x);
  $y = $id($y);
  return tuple($x,$y);
}

function sub1(): (string,int) {
  $f = function<T>(T $x): T ==> $x;
  return fork($f,"hello",42);
}

function sub2(): (string,int) {
  $f = function<<<__NoAutoBound>> T>(T $x): T ==> $x;
  return fork($f,"hello",42);
}
