<?hh

function main() {
  $a = miarray(
    1 => 'b',
    0 => 'a',
  );
  $res = array_fill_keys($a, 99);
  var_dump($res);
}

main();
