<?hh

function four() :mixed{ return 4; }
function heh() :mixed{ return darray['foo' => four()]; }
function bar() :mixed{ return darray['other' => heh()]; }
function foo() :mixed{
  $x = bar();
  $x['other']['foo'] = 2;
  return $x;
}
function main() :mixed{
  $x = foo();
  echo $x['other']['foo'] . "\n";
}

<<__EntryPoint>>
function main_array_016() :mixed{
main();
}
