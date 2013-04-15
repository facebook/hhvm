<?php
$arr = array(null, true, false, 0, 1, 0.0, 1.0, "", "foo", array(), array(1));

function id($x) {
  var_dump($x);
  return $x;
}

for ($i = 0; $i < count($arr); ++$i) {
  $x = $arr[$i];
  $y = id($x) ? id($x) : "blarg";
  echo "---------\n";
  var_dump($y);
  echo "\n\n";
}

echo "********************\n";

for ($i = 0; $i < count($arr); ++$i) {
  $x = $arr[$i];
  $y = id($x) ?: "blarg";
  echo "---------\n";
  var_dump($y);
  echo "\n\n";
}


