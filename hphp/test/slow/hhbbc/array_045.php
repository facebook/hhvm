<?hh

function foo() :mixed{
  return mt_rand() ? varray[1,2,3] : varray[2,3,4];
}
function bar() :mixed{
  $x = darray(foo());
  $x[123] = 2;
  $x['asdasdasd'] = new stdClass;
  return $x;
}
function main() :mixed{
  $x = bar();
  var_dump($x['asdasdasd']);
}

<<__EntryPoint>>
function main_array_045() :mixed{
main();
}
