<?hh
 <<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

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

function id_five_to_three<T as Three super Five>(T $x): T { return $x; }

function sub1(): (Three,Four) {
  $f = id_five_to_three<>;
  return fork($f,new Three(),new Four());
}

function id_three_to_one<T as One super Three>(T $x): T { return $x; }

function sub2(): (Two,Three) {
  $f = id_three_to_one<>;
  return fork($f,new Two(),new Three());
}
