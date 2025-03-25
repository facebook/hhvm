<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

function takePoly((function<T>(T): T) $f): void {
}

function callPoly((function<T>(T):T) $f): void {
  takePoly($f);
}
