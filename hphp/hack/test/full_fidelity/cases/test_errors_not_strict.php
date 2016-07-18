<?hh
function foo($a) {
  return $a;
}
function bar() : void {
  // No error; the annotations are not required on anonymous functions.
  $x = function($b) { };
  $y = <foo:bar:blah baz-abc="123"/>; // error on baz-abc
}
