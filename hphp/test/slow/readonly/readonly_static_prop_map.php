<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class Bar {
  public int $x = 3;
}

class Foo {
  public static readonly Map<string, Bar> $x = Map {};
}

<<__EntryPoint>>
function test(): void {
  $z = readonly new Bar();
  Foo::$x["a"] = $z; // error, must be readonly COW
}
