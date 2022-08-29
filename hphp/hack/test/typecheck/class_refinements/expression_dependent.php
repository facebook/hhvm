<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type TData as arraykey;
}

function getfun<T as arraykey>(Box with { type TData = T } $b): T {
  while(true) {}
}

function test(Box $b): arraykey {
  //
  // Validating the following call requires realizing that
  // $b can be given a concrete class type ?X such that
  // ?X <: Box. Given that ?X is concrete it must define
  // a TData member and the call should be permitted.
  //
  // The current machinery for this reasoning is called
  // expression-dependent types
  //
  $x = getfun($b);
  // hh_show($x);
  return $x;
}
