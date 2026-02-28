<?hh

function foo() :mixed{
  var_dump($x);
  if (is_string($x)) {
    $y = "asd";
  } else {
    $y = "asd2";
  }

  var_dump($y);
}


<<__EntryPoint>>
function main_jmp_local_008() :mixed{
foo();
}
