<?hh

class Foo {}
function foo($x) :mixed{
  if ($x) {
    $a = vec[];
    $s = 'hello';
    $o = new Foo();
  }
  try {
    var_dump((string)$a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump((string)$s);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump((string)$o);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1438() :mixed{
  foo(false);
}
