<?php
$str = "ABCDEFGHIJK";
$arr = array(null, false, true, 3, 4.0, 5.3, 6.7, -2, -2.5, 21, 22.5,
             PHP_INT_MAX, '', '8', '9a', 'foo', '1 ', ' 2', " \t2", " \n2",
             '999999999999999999999999999', '1.0 ', '2.0 ', '1e1', '1e1 ',
             ' 1e1', array(), array(1), new stdclass);
foreach ($arr as $x) {
  var_dump($x);
  var_dump($str[$x]);
  var_dump(isset($str[$x]));
  var_dump(!empty($str[$x]));
  echo "\n";
}

