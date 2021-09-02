<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {
  public function __construct(public (function(): void) $prop) {}
  <<__NEVER_INLINE>>
  public readonly function foo() : void {}
  <<__NEVER_INLINE>>
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
