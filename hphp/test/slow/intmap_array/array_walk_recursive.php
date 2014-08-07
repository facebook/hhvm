<?hh

function main() {
  $arr = miarray(
    0 => 1,
    1 => 2,
    3 => array(33 => 8),
    4 => 16,
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
