<?hh

class Foo {
  public int $prop = 4;
}

function foo(readonly dict<int, Foo> $v) : void {
  $y = readonly idx_readonly($v, 0) as nonnull;
  echo $y->prop . "\n";
}

<<__EntryPoint>>
function test(): void {
  $result = dict[0 => readonly new Foo()];
  foo($result);
  echo "Done!\n";
}
