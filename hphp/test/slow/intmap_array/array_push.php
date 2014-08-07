<?hh

function main() {
  $a = miarray(
    0 => 1,
    1 => 2,
    2 => 4,
  );
  $res = array_push($a, 8);
  var_dump($res);
  var_dump($a);
}

main();
