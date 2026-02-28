<?hh

class Foo {
  public static function blah() :mixed{ return true; }
}

function what($k) :mixed{}

<<__EntryPoint>> function a(): void {
  $x = Foo::blah() ? vec[1,2,3] : null;
  what($x);
}
