<?hh // strict
function foo($a) { // two errors; missing annotation on $a and foo.
  return $a;
}
function bar() : void {
  // No error; the annotations are not required on anonymous functions.
  $x = function($b) { };
  $y = <foo:bar:blah baz-abc="123"/>; // error on baz-abc
}
class C
{
  public $x = 123; // error; type annotation is not optional in strict mode
  public int $y = 456; // no error
}
function & baz(): void { // error: reference methods are not allowed in strict
                         // mode
}
