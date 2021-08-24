<?hh

<<file: __EnableUnstableFeatures('readonly')>>

function f($x): readonly int { echo ""; return $x; }

<<__EntryPoint>>
function main() {
  readonly f(1);
  try { f(1); } catch (Exception $e) { echo $e->getMessage()."\n"; }

  $f = __hhvm_intrinsics\launder_value('f');
  readonly $f(1);
  try { $f(1); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
