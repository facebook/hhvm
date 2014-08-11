<?hh

function main() {
  $arr = msarray(
    'a' => 1,
    'b' => 2,
    'c' => 4,
    'd' => 8,
    'e' => 16,
  );
  $res = array_splice($arr, 2, 1, array('lolwat'));
  var_dump($res);
  var_dump($arr);
  $res[] = "foo";
  $arr[] = "bar";
  var_dump($res);
  var_dump($arr);
}

main();
