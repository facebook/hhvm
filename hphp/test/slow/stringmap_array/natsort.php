<?hh

function main() {
  $arr = msarray(
    'c' => 99,
    'd' => 100,
    'z' => 123,
    'a' => 97,
    'b' => 98,
    'A' => 65,
  );
  $res = natsort($arr);
  var_dump($res);
  var_dump($arr);
}

main();
