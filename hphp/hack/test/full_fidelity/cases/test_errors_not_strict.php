<?hh
function foo($a) {
  return $a;
}
function bar() : void {
  // No error; the annotations are not required on anonymous functions.
  $x = function($b) { };
  $y = <foo:bar:blah baz-abc="123"/>; // error on baz-abc
}
class C
{
  public $x = 123; // no error; type annotation is optional
  public int $y = 456; // no error
}
function & baz(): void { // no error: reference methods are allowed in
                         // non-strict mode
}
