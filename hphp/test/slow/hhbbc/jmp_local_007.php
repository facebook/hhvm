<?hh

function foo() :mixed{
  if ($x) {
    $y = "asd";
  } else {
    $y = "asd2";
  }

  var_dump($y);
}


<<__EntryPoint>>
function main_jmp_local_007() :mixed{
foo();
}
