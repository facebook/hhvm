<?hh

function main() {
  $arr = msarray(
    'a' => 1,
    'b' => 2,
    'c' => 4,
    'd' => 8,
    'e' => 16,
  );
  $res = array_walk(
    $arr,
    function($key, &$val) {
      $val = 0;
    });
  var_dump($res);
  var_dump($arr);
}

main();
