<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints','polymorphic_lambda')>>

class One {}
class Two extends One {}
class Three extends Two {}
class Four extends Three {}
class Five extends Four {}

function fork<
  T1 as Two super Four,
  T2 as Three super Four
 >(
  (function<T as Two super Four>(T): T) $id,
  T1 $x,
  T2 $y
): (T1,T2) {
  var_dump($id);
  $x = $id($x);
  $y = $id($y);
  return tuple($x,$y);
}

<<__EntryPoint>>
function main(): void {
  $f = function<T>(T $x): T ==> $x;
  var_dump($f);
  $_ = fork($f,new Three(),new Four());
  $g = function<T as One super Five>(T $x): T ==> $x;
  var_dump($g);
  $_ = fork($g,new Three(),new Four());
}
