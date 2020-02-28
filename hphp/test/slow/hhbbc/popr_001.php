<?hh

class A {}

function bar(inout $a) {
  return $a[0];
}

function main() {
  $a = varray[new A];
  bar(inout $a);
}


<<__EntryPoint>>
function main_popr_001() {
main();
}
