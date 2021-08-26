<?hh

<<file:__EnableUnstableFeatures('readonly')>>

class Bar {
  public int $prop;
}
class Foo {
  public static readonly Bar $c;
}

<<__EntryPoint>>
function test(): void {
  Foo::$c = new Bar();
  Foo::$c->prop = 10; // must be mutable
}
