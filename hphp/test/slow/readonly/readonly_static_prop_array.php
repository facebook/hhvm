<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class Bar {
  public int $x = 3;
}

class Foo {
  public static readonly dict<string, Bar> $x = dict[];
}

<<__EntryPoint>>
function test(): void {
  $z = readonly new Bar();
  Foo::$x["a"] = $z;
  echo "Done!\n";
}
