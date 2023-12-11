<?hh

function foo() :mixed{ return vec[1,2,3]; }
function bar() :mixed{ return vec[]; }
function test($x) :mixed{
  $y = $x ? foo() : bar();
  return $y[0];
}
<<__EntryPoint>>
function main() :mixed{
  try { var_dump(test(true)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { var_dump(test(false)); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
