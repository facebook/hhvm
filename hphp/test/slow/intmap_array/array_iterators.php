<?hh

function main() {
  $arr = miarray(
    1 => 'a',
    2 => 'b',
    3 => 'c',
    4 => 'd',
    5 => 'e',
    26 => 'z',
  );
  $res = each($arr);
  while ((bool)$res) {
    var_dump($res);
    $res = each($arr);
  }
  $res = reset($arr);
  var_dump(key($arr));
  var_dump(current($arr));
  $res = next($arr);
  var_dump($res);
  $res = prev($arr);
  var_dump($res);
  $res = end($arr);
  var_dump($res);
  var_dump($arr);
  $arr[] = "warning";
}

main();
