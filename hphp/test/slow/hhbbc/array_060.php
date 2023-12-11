<?hh

function foo() :mixed{
  $x = dict[0 => 1];
  for ($i = 0; $i < 2; ++$i) {
    $x[$i] = 'a';
  }
  if ($x ?? false) {
    echo "not empty\n";
  } else {
    echo "hm\n";
  }
}


<<__EntryPoint>>
function main_array_060() :mixed{
foo();
}
