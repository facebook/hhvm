<?hh // strict

// This is a regression test to ensure that when we add a bound to a type
// parameter, we don't accidentally delete the information about where it was
// defined

function test<T as num>(mixed $x) : void {
  // Make sure that the definition site of T is shown correctly in arity error
  if ($x is T<int>) {}
}
