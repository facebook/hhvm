<?hh

function x($x) :mixed{
  $z = new stdClass;
  $k = new stdClass;
  $p = new stdClass;
  $l = new stdClass;
  hash($x, $x);
  return $x;
}


<<__EntryPoint>>
function main_refcount_callbuiltin() :mixed{
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
}
