<?hh

function main() {
  $a = msarray(
    'a' => null,
    'b' => 98,
    'c' => 99,
  );
  $res = array_filter($a);
  var_dump($res);
}

main();
