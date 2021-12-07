<?hh

class Foo {
  public function __construct(public (function(): void) $prop)[] {}
  public readonly function foo() : void {}
  public function bar(): void {}
}

<<__EntryPoint>>
function test() : void {
  $f = readonly new Foo(() ==> {echo "hi\n";});
  $f->foo(); // ok
  try {
    $f->bar(); // TODO: error, $f is readonly\
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  echo "Done\n";
}
