<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Box<T> {}

function takePoly(
  (function<T1 as Box<T2>, T2 as Box<T2>>(T1, T2): void) $f,
): (function<T1 as Box<T2>, T2 as Box<T2>>(T1, T2): void) {
  return $f;
}
