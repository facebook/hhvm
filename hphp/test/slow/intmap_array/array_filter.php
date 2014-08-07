<?hh

function main() {
  $a = miarray(
    5 => null,
    10 => 11,
    15 => 16,
  );
  $res = array_filter($a);
  var_dump($res);
}

main();
