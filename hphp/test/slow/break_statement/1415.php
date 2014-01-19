<?php

$arr = array('one', 'two', 'three', 'four', 'stop', 'five');
while (list(, $val) = each($arr)) {
  if ($val == 'stop') {
    break;
  }
  echo "$val\n";
}
$i = 0;
while (++$i) {
    switch ($i) {
    case 5:
      echo "At 5\n";
      break 1;
    case 10:
      echo "At 10; quitting\n";
      break 2;
    default:
      break;
    }
}
