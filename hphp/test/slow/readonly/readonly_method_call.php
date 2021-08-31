<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {
  public function __construct(public (function(): void) $prop) {}
  public readonly function foo() : void {}
  public function bar(): void {}
}

<<__EntryPoint>>
function test() : void {
  $f = readonly new Foo(() ==> {echo "hi\n";});
  $f->foo(); // ok
  $f->bar(); // TODO: error, $f is readonly\
  echo "Done\n";
}
