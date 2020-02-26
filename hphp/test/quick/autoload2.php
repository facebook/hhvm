<?hh
function __autoload($cls) {
  echo "__autoload $cls\n";
  if ($cls === 'C') {
    include 'autoload1.inc';
  }
}
$arr = varray["C"];
$obj = new $arr[0];

