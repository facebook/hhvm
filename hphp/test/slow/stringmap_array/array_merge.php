<?hh

function main() {
  $empty = array();
  $msarr = msarray(
    'one' => 2,
    'bonjour' => 10,
  );
  $res = array_merge($empty, $msarr);
  var_dump($res);
  $res = array_merge($empty, $empty, $msarr);
  var_dump($res);
  $res = array_merge($empty, $empty, $msarr, $empty);
  var_dump($res);
  $res = array_merge($msarr, $msarr);
  var_dump($res);
  $res = array_merge($msarr, $msarr, $msarr);
  var_dump($res);
  $res = array_merge($msarr, $empty);
  var_dump($res);
  $res = array_merge($msarr, $empty, $empty);
  var_dump($res);
}

main();
