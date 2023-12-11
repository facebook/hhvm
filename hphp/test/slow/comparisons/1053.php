<?hh

class C {
}
function foo($p) :mixed{
  if ($p) {
    $obj = new C;
  } else {
    $a = vec[1];
  }
  try {
    var_dump($obj == $a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1053() :mixed{
  foo(false);
}
