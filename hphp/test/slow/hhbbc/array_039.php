<?hh

class C { function heh() { echo "heh\n"; } }
function foo() { return varray[]; }
function bar() {
  $x = foo();
  $x['a'] = new C;
  return $x;
}
function main() {
  $y = bar()['a'];
  $y->heh();
}

<<__EntryPoint>>
function main_array_039() {
main();
}
