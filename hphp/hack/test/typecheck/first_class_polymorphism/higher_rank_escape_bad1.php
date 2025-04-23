<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Box<T> {}

function takePoly((function<T>(Box<T>): void) $g): void {}

function passWithTvar(): void {
  $g = (Box<_> $_): void ==> {};
  takePoly($g);
}
