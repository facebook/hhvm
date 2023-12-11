<?hh

class C { function heh() :mixed{ echo "heh\n"; } }
function foo() :mixed{ return dict[]; }
function bar() :mixed{
  $x = foo();
  $x['a'] = new C;
  return $x;
}
function main() :mixed{
  $y = bar()['a'];
  $y->heh();
}

<<__EntryPoint>>
function main_array_039() :mixed{
main();
}
