<?hh

function main() {
  $arr1 = msarray(
    'b' => 98,
    'a' => 97,
  );
  $arr2 = msarray(
    'd' => 100,
    'c' => 99,
  );
  $res = array_multisort($arr1, $arr2);
  var_dump($res);
  var_dump($arr1);
  var_dump($arr2);
}

main();
