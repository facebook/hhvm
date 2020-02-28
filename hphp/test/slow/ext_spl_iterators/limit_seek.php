<?hh

function check_seek($lim, $seek) {
  try {
    $lim->seek($seek);
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_limit_seek() {
$arr = new ArrayIterator(varray['a','b','c']);

$lim1 = new LimitIterator($arr, 0, 2);
check_seek($lim1, 0);
check_seek($lim1, 1);
check_seek($lim1, 2);

$lim2 = new LimitIterator($arr, 1);
check_seek($lim2, 0);
check_seek($lim2, 1);
check_seek($lim2, 2);
}
