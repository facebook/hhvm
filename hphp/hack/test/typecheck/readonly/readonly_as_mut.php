<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class Foo {
  public function __construct(public int $prop = 4) {
  }
}

function takes_int(int $x) : void {
  echo $x;
}

<<__EntryPoint>>
function test_stuff() : void {
  $foo = readonly new Foo();
  // $x is readonly here by deepness
  $x = \HH\Readonly\as_mut($foo->prop);
  // should work
  takes_int($x);
}
