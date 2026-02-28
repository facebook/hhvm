<?hh
function main() :mixed{
  $arr = vec[null, false, true, 0, 1, 0.0, 1.0, '', '0', '1',
               '0.0', '1.0', '0 ', 'foo'];
  $vec = Vector::fromArray($arr);
  foreach ($vec as $k => $v) {
    var_dump($v);
    var_dump($vec[$k]);
    var_dump(isset($v));
    var_dump(isset($vec[$k]));
    var_dump(!($v ?? false));
    var_dump(!($vec[$k] ?? false));
    echo "\n";
  }
  echo "------------------------\n\n";
  $mp = Map::fromArray($arr);
  foreach ($arr as $k => $v) {
    var_dump($mp[$k]);
    var_dump(isset($mp[$k]));
    var_dump(!($mp[$k] ?? false));

    echo "\n";
  }
  $new_arr = dict[];
  foreach ($mp as $k => $v) {
    $new_arr[$k] = $v;
  }
  ksort(inout $new_arr);
  var_dump($new_arr);
  echo "Done\n";
}

<<__EntryPoint>>
function main_802() :mixed{
main();
}
