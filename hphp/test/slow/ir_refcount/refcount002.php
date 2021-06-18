<?hh

function foo() { mt_rand(); mt_rand(); mt_rand(); return new stdClass(); }

function bar(inout $k, inout $z) {
  $y = foo();
  echo $z;
}


<<__EntryPoint>>
function main_refcount002() {
  $k = "asd";
  bar(inout $k, inout $k);
}
