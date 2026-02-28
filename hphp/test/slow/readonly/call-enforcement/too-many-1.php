<?hh

<<file: __EnableUnstableFeatures('readonly')>>

class C {}

function f(int $arg) :mixed{
  echo "in f\n";
}

<<__EntryPoint>>
function main() :mixed{
  $ro = readonly new C();
  try { f($ro, $ro, $ro); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $f = __hhvm_intrinsics\launder_value('f');
  try { $f($ro, $ro, $ro); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
