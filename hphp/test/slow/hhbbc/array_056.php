<?hh

class C { public function heh() { echo "hey\n"; } }
function foo() {
  $x = varray[];
  for ($i = 0; $i < 10; ++$i) {
    $x[] = new C;
  }
  return $x;
}
function main() {
  $x = foo();
  $x[0]->heh();
}

<<__EntryPoint>>
function main_array_056() {
main();
}
