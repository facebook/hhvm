<?hh

function four() { return 4; }
function heh2() { return darray['heh' => four()]; }
function heh() { return darray['foo' => heh2()]; }
function bar() { return darray['other' => heh()]; }
function foo() {
  $x = bar();
  $x['other']['foo']['heh'] = 2;
  $x['other']['whatever'] = 2;
  $x['yoyo'] = darray['stuff' => $x];
  return $x;
}
function main() {
  $x = foo();
  echo $x['other']['foo']['heh'] . "\n";
  echo $x['other']['whatever'] . "\n";
  var_dump($x['yoyo']['stuff']['other']['foo']['heh']);
  var_dump($x['yoyo']['stuff']);
}

<<__EntryPoint>>
function main_array_017() {
main();
}
