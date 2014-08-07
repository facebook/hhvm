<?hh

function main() {
  $arr = msarray(
    'a' => 97,
    'b' => 98,
    'c' => 99,
    'd' => 100,
    'z' => 123,
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
  var_dump($arr);
  $arr[] = "warning";
}

main();
