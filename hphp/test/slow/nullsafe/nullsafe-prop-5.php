<?hh

function byVal($x) :mixed{
  echo 'byVal is called, $x is: ';
  var_dump($x);
}

function test() :mixed{
  $x = null;
  byVal($x?->y); // ok
}


<<__EntryPoint>>
function main_nullsafe_prop_5() :mixed{
test();
}
