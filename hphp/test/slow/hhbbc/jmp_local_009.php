<?hh

class A {}

function foo() {
  try {
    if ($x is A) {
      $y = "asd";
    } else {
      $y = "asd2";
    }
    var_dump($y);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_jmp_local_009() {
  foo();
}
