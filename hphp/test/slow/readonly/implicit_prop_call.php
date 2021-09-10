<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {
  public function __construct(public (function(): void) $prop) {}
}

<<__EntryPoint>>
function test() : void {
  $f = readonly new Foo(() ==> {echo "hi\n";});
  try {
    ($f->prop)(); // fail, $f is readonly so the function needs to be readonly
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
