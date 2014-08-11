<?hh

function main() {
  $arr1 = miarray(
    5 => 'b',
    4 => 'a',
  );
  $arr2 = miarray(
    3 => 'd',
    2 => 'c',
  );
  $res = array_multisort($arr1, $arr2);
  var_dump($res);
  var_dump($arr1);
  var_dump($arr2);
}

main();
