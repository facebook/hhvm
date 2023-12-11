<?hh

function heh() :mixed{ return 4; }
function bar() :mixed{ return dict['foo' => heh()]; }
function foo() :mixed{
  $x = bar();
  $x['foo'] = 2;
  return $x;
}
function main() :mixed{
  $x = foo();
  echo $x['foo'] . "\n";
}

<<__EntryPoint>>
function main_array_014() :mixed{
main();
}
