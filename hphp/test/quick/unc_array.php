<?hh

class Foo {
  public static function blah() { return true; }
}

function what($k) {}

<<__EntryPoint>> function a(): void {
  $x = Foo::blah() ? varray[1,2,3] : null;
  what($x);
}
