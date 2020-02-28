<?hh

class C { function heh() { echo "heh\n"; } }
function foo() { return varray[]; }
function bar() {
  $x = foo();
  $x[0] = new C;
  return $x;
}
function main() {
  $y = bar()[0];
  $y->heh();
}

<<__EntryPoint>>
function main_array_040() {
main();
}
