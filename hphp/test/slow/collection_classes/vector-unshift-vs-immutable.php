<?hh

function unshiftAndDump($unshiftMe) {
  $v = Vector {1, 2, 3};
  var_dump($v->immutable());
  array_unshift($v, $unshiftMe);
  var_dump($v->immutable());
}

unshiftAndDump(0);
