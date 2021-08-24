<?hh

<<file: __EnableUnstableFeatures('readonly')>>

function f($x): readonly int { echo ""; return $x; }

<<__EntryPoint>>
function main() {
  readonly f(1);
  f(1);
  $f = __hhvm_intrinsics\launder_value('f');
  readonly $f(1);
  $f(1);
}
