<?hh

<<file: __EnableUnstableFeatures('readonly')>>

<<__DynamicallyCallable>> function f($x): readonly int { echo ""; return $x; }

<<__EntryPoint>>
function main() :mixed{
  readonly f(1);
  try { f(1); } catch (ReadonlyViolationException $e) { echo $e->getMessage()."\n"; }

  $f = __hhvm_intrinsics\launder_value('f');
  readonly HH\dynamic_fun($f)(1);
  try { HH\dynamic_fun($f)(1); } catch (ReadonlyViolationException $e) { echo $e->getMessage()."\n"; }
}
