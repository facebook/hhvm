<?hh

function main() {
  $arr = msarray(
    'one' => 1,
    'two' => 2,
    'eighty-five' => 85,
  );
  $res = array_chunk($arr, 2);
  var_dump($res);

  $res = array_chunk($arr, 2, true);
  var_dump($res);
}

main();
