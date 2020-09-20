<?hh

function unshiftAndDump($unshiftMe) {
  $v = Vector {1, 2, 3};
  var_dump($v->immutable());
  array_unshift(inout $v, $unshiftMe);
  var_dump($v->immutable());
}


<<__EntryPoint>>
function main_vector_unshift_vs_immutable() {
unshiftAndDump(0);
}
