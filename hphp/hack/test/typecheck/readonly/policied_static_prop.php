<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class Foo {
  public static ?Foo $x = null;
}

function foo_policied()[policied] : void {
  $y = new Foo();
  $v = readonly $y::$x; // ok
  $z = $y::$x; // not ok
  // not ok
  $y::$x = new Foo();
}
