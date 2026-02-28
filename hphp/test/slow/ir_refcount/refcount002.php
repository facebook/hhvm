<?hh

function foo() :mixed{ mt_rand(); mt_rand(); mt_rand(); return new stdClass(); }

function bar(inout $k, inout $z) :mixed{
  $y = foo();
  echo $z;
}


<<__EntryPoint>>
function main_refcount002() :mixed{
  $k = "asd";
  bar(inout $k, inout $k);
}
