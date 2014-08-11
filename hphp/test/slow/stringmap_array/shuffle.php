<?hh

function main() {
  $arr = msarray(
    'a' => 97,
    'b' => 98,
    'c' => 99,
    'd' => 100,
    'e' => 101,
  );
  $res = shuffle($arr);
  var_dump($res);
  $arr[] = 'no warn';
}

main();
