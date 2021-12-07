<?hh

class Foo {
  public function __construct(public (function(): void) $prop)[] {}
}

<<__EntryPoint>>
function test() : void {
  $f = new Foo(() ==> {echo "hi\n";});
  (readonly $f->prop)(); // fail, $f is not marked readonly
  echo "Done\n";
}
