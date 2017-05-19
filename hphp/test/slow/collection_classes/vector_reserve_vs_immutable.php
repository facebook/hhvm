<?hh

function main($v) {
  $v[] = 4;
  $iv = $v->toImmVector();
  $v->reserve(1);
  var_dump($v->toImmVector());
}

main(Vector {1, 2, 3});
