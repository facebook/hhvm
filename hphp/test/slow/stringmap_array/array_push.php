<?hh

function main() {
  $a = msarray(
    'a' => 96,
    'b' => 97,
    'c' => 98,
  );
  $res = array_push($a, 99);
  var_dump($res);
  var_dump($a);
}

main();
