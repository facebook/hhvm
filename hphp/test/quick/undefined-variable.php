<?hh
function foo() :mixed{
  if (\HH\global_isset('b')) $b = 0;
  try {
    return $b;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

function baz($x) :mixed{}
function bar() :mixed{
  if (\HH\global_isset('a')) $a = 0;
  try {
    baz($a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main() :mixed{
  foo();
  bar();
}
