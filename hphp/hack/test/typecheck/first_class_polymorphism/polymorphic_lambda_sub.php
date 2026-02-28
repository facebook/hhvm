<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

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
  $x = $id($x);
  $y = $id($y);
  return tuple($x,$y);
}

function sub1(): (Three,Four) {
  $f = function<T>(T $x): T ==> $x;
  return fork($f,new Three(),new Four());
}


function sub2(): (Three,Four) {
  $f =  function<T as One super Five>(T $x): T ==> $x;
  return fork($f,new Three(),new Four());
}
