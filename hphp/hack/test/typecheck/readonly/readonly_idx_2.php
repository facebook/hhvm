<?hh
<<file:__EnableUnstableFeatures("readonly")>>


class Foo {
  public int $prop = 4;
}

function foo(readonly dict<int, Foo> $v) : void {
  $y = readonly idx_readonly($v, 5) as nonnull; // without default should return nullable, so nonnull
  $y->prop = 5;
}
