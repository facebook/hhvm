<?hh
<<file:__EnableUnstableFeatures("readonly")>>


class Foo {
  public int $prop = 4;
}

function foo(readonly dict<int, Foo> $v) : void {
  $z = new Foo();
  $z->prop = 7;
  $y = readonly idx_readonly($v, 2, $z);
  echo $y->prop . "\n";
}

<<__EntryPoint>>
function test(): void {
  $result = dict[0 => readonly new Foo()];
  foo($result);
  echo "Done!\n";
}
