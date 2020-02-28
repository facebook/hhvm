<?hh

function foo() { return varray[1,2,3]; }
function bar() { return varray[]; }
function test($x) {
  $y = $x ? foo() : bar();
  return $y[0];
}
<<__EntryPoint>>
function main() {
  try { var_dump(test(true)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { var_dump(test(false)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
