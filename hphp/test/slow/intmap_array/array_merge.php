<?hh

function main() {
  $empty = array();
  $miarr = miarray(
    1 => 2,
    -7 => 10,
  );
  $res = array_merge($empty, $miarr);
  var_dump($res);
  $res = array_merge($empty, $empty, $miarr);
  var_dump($res);
  $res = array_merge($empty, $empty, $miarr, $empty);
  var_dump($res);
  $res = array_merge($miarr, $miarr);
  var_dump($res);
  $res = array_merge($miarr, $miarr, $miarr);
  var_dump($res);
  $res = array_merge($miarr, $empty);
  var_dump($res);
  $res = array_merge($miarr, $empty, $empty);
  var_dump($res);
}

main();
