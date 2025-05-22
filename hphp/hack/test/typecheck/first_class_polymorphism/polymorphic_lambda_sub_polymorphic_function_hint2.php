<?hh

<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

function return_poly1<T1>(T1 $x): (function<T2>(T2):(T1,T2)) {
  $f = (function<T>(T $y): (T1,T) ==> tuple($x, $y));
  return $f;
}

function return_poly2<T1>(T1 $x): (function<T2>(T2):(T1,T2)) {
  return (function<T>(T $y): (T1,T) ==> tuple($x, $y));
}
