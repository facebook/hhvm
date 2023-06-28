<?hh

function four() :mixed{ return 4; }
function heh2() :mixed{ return darray['heh' => four()]; }
function heh() :mixed{ return darray['foo' => heh2()]; }
function bar() :mixed{ return darray['other' => heh()]; }
function foo() :mixed{
  $x = bar();
  $x['other']['foo']['heh'] = 2;
  $x['other']['whatever'] = 2;
  $x['yoyo'] = darray['stuff' => $x];
  return $x;
}
function main() :mixed{
  $x = foo();
  echo $x['other']['foo']['heh'] . "\n";
  echo $x['other']['whatever'] . "\n";
  var_dump($x['yoyo']['stuff']['other']['foo']['heh']);
  var_dump($x['yoyo']['stuff']);
}

<<__EntryPoint>>
function main_array_017() :mixed{
main();
}
