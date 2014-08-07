<?hh

function main() {
  $arr = miarray(
    1 => 'one',
    2 => 'two',
    85 => 'eighty-five',
  );
  $res = array_chunk($arr, 2);
  var_dump($res);

  $res = array_chunk($arr, 2, true);
  var_dump($res);
}

main();
