<?hh

function makeOptVec(): ~?vec<int> {
  return null;
}

<<__SupportDynamicType>>
function foo<Tv1 as supportdyn<mixed>>(Traversable<Tv1> $traversable): void {}

function test(): void {
  $options = makeOptVec();
  $x = $options ?? vec[];
  foo($x);
  foo<int>($options ?? vec[]);
  foo($options ?? vec[]);
}
