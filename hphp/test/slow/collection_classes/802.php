<?hh
function main() {
  $arr = array(null, false, true, 0, 1, 0.0, 1.0, '', '0', '1',
               '0.0', '1.0', '0 ', 'foo');
  $vec = Vector::fromArray($arr);
  foreach ($vec as $k => $v) {
    var_dump($v);
    var_dump($vec[$k]);
    var_dump(isset($v));
    var_dump(isset($vec[$k]));
    var_dump(empty($v));
    var_dump(empty($vec[$k]));
    echo "\n";
  }
  echo "------------------------\n\n";
  $mp = Map::fromArray($arr);
  foreach ($arr as $k => $v) {
    var_dump($mp[$k]);
    var_dump(isset($mp[$k]));
    var_dump(empty($mp[$k]));

    echo "\n";
  }
  $new_arr = array();
  foreach ($mp as $k => $v) {
    $new_arr[$k] = $v;
  }
  ksort($new_arr);
  var_dump($new_arr);
  echo "------------------------\n\n";
  $smp = StableMap::fromArray($arr);
  foreach ($smp as $k => $v) {
    var_dump($v);
    var_dump($smp[$k]);
    var_dump(isset($v));
    var_dump(isset($smp[$k]));
    var_dump(empty($v));
    var_dump(empty($smp[$k]));
    echo "\n";
  }
  echo "Done\n";
}
main();
