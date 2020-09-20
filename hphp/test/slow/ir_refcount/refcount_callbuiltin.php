<?hh

function x($x) {
  $z = new stdclass;
  $k = new stdclass;
  $p = new stdclass;
  $l = new stdclass;
  hash($x, $x);
  return $x;
}


<<__EntryPoint>>
function main_refcount_callbuiltin() {
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
}
