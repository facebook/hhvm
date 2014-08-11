<?hh

function main() {
  $arr = miarray(
    0 => 1,
    1 => 2,
    2 => 4,
    3 => 8,
    4 => 16,
  );
  $res = array_splice($arr, 2, 1);
  var_dump($res);
  var_dump($arr);
  $res[] = "foo";
  $arr[] = "bar";
  var_dump($res);
  var_dump($arr);
}

main();
