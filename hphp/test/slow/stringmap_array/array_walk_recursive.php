<?hh

function main() {
  $arr = msarray(
    'a' => 1,
    'b' => 2,
    'c' => array('cc' => 4),
    'd' => 8,
    'e' => 16,
  );
  $res = array_walk_recursive(
    $arr,
    function(&$val, $key) {
      $val = 0;
    });
  var_dump($res);
  var_dump($arr);
}

main();
