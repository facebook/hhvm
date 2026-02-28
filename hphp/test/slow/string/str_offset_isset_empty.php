<?hh

function _try($fn) :mixed{
  try {
    var_dump($fn());
  } catch (Exception $e) {
    echo get_class($e).': '.$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $str = "ABCDEFGHIJK";
  $arr = vec[null, false, true, 3, 4.0, 5.3, 6.7, -2, -2.5, 21, 22.5,
              PHP_INT_MAX, '', '8', '9a', 'foo', '1 ', ' 2', " \t2", " \n2",
              '999999999999999999999999999', '1.0 ', '2.0 ', '1e1', '1e1 ',
              ' 1e1', vec[], vec[1], new stdClass, 0x100000004];
  foreach ($arr as $x) {
    var_dump($x);
    _try(() ==> $str[$x]);
    try {
      var_dump(isset($str[$x]));
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    echo "\n";
  }
}
