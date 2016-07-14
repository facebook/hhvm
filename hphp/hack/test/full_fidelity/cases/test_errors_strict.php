<?hh // strict
function foo($a) {
  return $a;
}
function bar() : void {
  // No error; the annotations are not required on anonymous functions.
  $x = function($b) { };
  $y = <foo:bar:blah baz-abc="123"/>;
  case 123: ;
  default: ;
  switch ($x) { case 123: ; default: ; }
}
