<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>


function fork(
  (function<<<__NoAutoBound>> T>(T): T) $id,
  string $x,
  int $y
): (string,int) {
  $x = $id($x);
  $y = $id($y);
  return tuple($x,$y);
}

function id<T>(T $x): T { return $x; }

function id_no_auto_bound<<<__NoAutoBound>> T>(T $x): T { return $x; }

function sub1(): (string,int) {
  $f = id<>;
  return fork($f,"hello",42); // Error under implicit pessimisation
}

function sub2(): (string,int) {
  $f = id_no_auto_bound<>;
  return fork($f,"hello",42);
}
