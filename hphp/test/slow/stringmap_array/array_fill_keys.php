<?hh

function main() {
  $a = msarray(
    'a' => 1,
    'b' => 0,
  );
  $res = array_fill_keys($a, 99);
  var_dump($res);
}

main();
