<?hh

function main() {
  $arr = miarray(
    4 => 'd',
    5 => 'e',
    -5 => 'q',
    52 => 'A',
    26 => 'z',
    1 => 'a',
    2 => 'b',
    3 => 'c',
  );
  $res = natsort($arr);
  var_dump($res);
  var_dump($arr);
}

main();
