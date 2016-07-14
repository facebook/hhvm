<?hh // strict
function foo($a) {
  return $a;
}
function bar() : void {
  // No error; the annotations are not required on anonymous functions.
  $x = function($b) { };
  $y = <foo:bar:blah baz-abc="123"/>; // error on baz-abc
  case 123: ; // error, outside switch
  default: ; // error, outside switch
  switch ($x) { case 123: ; default: break;} // no error
  break; // error
  while($a) {
    if ($b) break; // no error
    if ($c) continue; // no error
    $x = function($b) { break; }; // error
  }
  try {} catch {} // no error
  try {} finally {} // no error
  try {} // error
}
