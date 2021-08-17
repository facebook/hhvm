<?hh

<<file: __EnableUnstableFeatures('readonly')>>

class C {}

function f($x) { echo "in f\n"; }
function g(readonly $x, $y) { echo "in g\n"; }
function h(...$x) { echo "in h\n"; }

<<__EntryPoint>>
function main() {
  $mut = new C();
  $ro = readonly new C();

  f($mut);
  f($ro);

  g($mut, $mut);
  g($ro, $mut);
  g($mut, $ro);
  g($ro, $ro);

  h($ro, $ro);
}
