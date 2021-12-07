<?hh

class Foo {
  public function __construct(public (function(): void) $prop)[] {}
}

<<__EntryPoint>>
function test() : void {
  $f = new Foo(() ==> {echo "hi\n";});
  try {
    (readonly $f->prop)(); // fail, $f is not marked readonly
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  echo "Done\n";
}
