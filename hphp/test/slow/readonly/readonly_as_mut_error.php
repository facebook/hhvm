<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class Foo {
  public function __construct() {
  }
}

<<__EntryPoint>>
function test_stuff() : void {
  $foo = readonly new Foo();
  // error, not a value type
  $x = \HH\Readonly\as_mut($foo);
}
