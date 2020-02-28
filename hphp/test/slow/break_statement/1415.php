<?hh


<<__EntryPoint>>
function main_1415() {
$arr = varray['one', 'two', 'three', 'four', 'stop', 'five'];
while (list(, $val) = each(inout $arr)) {
  if ($val == 'stop') {
    break;
  }
  echo "$val\n";
}
$i = 0;
$valid = true;
while ($valid && ++$i) {
    switch ($i) {
    case 5:
      echo "At 5\n";
      break;
    case 10:
      echo "At 10; quitting\n";
      $valid = false;
      break;
    default:
      break;
    }
}
}
