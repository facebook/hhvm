<?hh

class C { function heh() { echo "hi\n"; } }
function foo() {
  $x = varray[new C, new C];
  foreach ($x as $v) {
    $v->heh();
  }
}

<<__EntryPoint>>
function main_iter_001() {
foo();
}
