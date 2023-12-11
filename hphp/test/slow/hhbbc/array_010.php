<?hh

function foo() :mixed{
  $x = null;
  for ($i = 0; $i < 1024; ++$i) {
    $x = dict['foo' => $x];
  }
}

<<__EntryPoint>>
function main_array_010() :mixed{
foo();
}
