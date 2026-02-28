<?hh

class C { function heh() :mixed{ echo "heh\n"; } }
function foo() :mixed{ return vec[]; }
function bar() :mixed{
  $x = foo();
  $x[] = new C;
  return $x;
}
function main() :mixed{
  $y = bar()[0];
  $y->heh();
}

<<__EntryPoint>>
function main_array_040() :mixed{
main();
}
