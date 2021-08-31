<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {
  public function __construct(public readonly (readonly function(): void) $prop) {}
}

<<__EntryPoint>>
function test() : void {
  $f = new Foo(() ==> {echo "hi\n";});
  (readonly $f->prop)(); // This is allowed, since $prop is marked readonly at declaration time
  echo "Done\n";
}
