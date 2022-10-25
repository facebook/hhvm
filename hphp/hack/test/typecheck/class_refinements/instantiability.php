<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

function f_good(classname<Box with { type T = int }> $_): void {}

function good<T as arraykey>(
  classname<Box with { type T = T }> $good1, // OK
): void {}

function bad<T as arraykey>(
  classname<?Box with { type T = T }> $bad1,
  classname<shape() with { type T = int }> $bad2,
): void {}
