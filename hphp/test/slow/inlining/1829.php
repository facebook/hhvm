<?hh

function foo($e='e') :mixed{
  return '<a name="'.$e.'" id="'.$e.'"></a>';
}
function test() :mixed{
  echo foo();
}

<<__EntryPoint>>
function main_1829() :mixed{
test();
}
