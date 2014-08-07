<?hh

function main() {
  $arr = miarray(
    0 => 1,
    1 => 2,
    3 => 8,
    4 => 16,
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
