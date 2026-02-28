<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Box<T> {}

function takePolyNested((function<T>(Box<Box<T>>): void) $g): void {}

function passWithTvar(): void {
  $g = (Box<_> $_): void ==> {};
  takePolyNested($g);
}
