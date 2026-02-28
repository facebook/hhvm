<?hh

<<file: __EnableUnstableFeatures('readonly')>>

class C {}

function f($x) :mixed{ echo "in f\n"; }
function g(readonly $x, $y) :mixed{ echo "in g\n"; }
function h(...$x) :mixed{ echo "in h\n"; }

<<__EntryPoint>>
function main() :mixed{
  $mut = new C();
  $ro = readonly new C();

  f($mut);
  try { f($ro); } catch (ReadonlyViolationException $e) { echo $e->getMessage()."\n"; }

  g($mut, $mut);
  g($ro, $mut);
  try { g($mut, $ro); } catch (ReadonlyViolationException $e) { echo $e->getMessage()."\n"; }
  try { g($ro, $ro); } catch (ReadonlyViolationException $e) { echo $e->getMessage()."\n"; }

  try { h($ro, $ro); } catch (ReadonlyViolationException $e) { echo $e->getMessage()."\n"; }
}
