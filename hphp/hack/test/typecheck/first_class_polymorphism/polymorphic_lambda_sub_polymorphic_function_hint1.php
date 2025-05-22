<?hh

<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

class One {}
class Two extends One {}
class Three extends Two {}

function return_poly1<T1 as One super Three>(T1 $x): (function<T2 as One super Two>(T2):(T1,T2)) {
  $f = (function<T as One super Three>(T $y): (T1,T) ==> tuple($x, $y));
  return $f;
}

function return_poly2<T1 as One super Three>(T1 $x): (function<T2 as One super Two>(T2):(T1,T2)) {
  return (function<T as One super Three>(T $y): (T1,T) ==> tuple($x, $y));
}
