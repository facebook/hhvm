<?hh

class C {
}
function foo($p) {
  if ($p) {
    $obj = new C;
  } else {
    $a = varray[1];
  }
  try {
    var_dump($obj == $a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1053() {
  foo(false);
}
