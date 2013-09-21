<?php

echo "*** Testing array_keys() on all the types other than arrays ***\n";
$types_arr = array(
  TRUE => TRUE,
  FALSE => FALSE,
  1 => 1,
  0 => 0,
  -1 => -1, 
  "1" => "1",
  "0" => "0",
  "-1" => "-1",
  NULL,
  array(),
  "php" => "php",
  "" => ""
);
$values = array(TRUE, FALSE, 1, 0, -1, "1", "0", "-1",  NULL, array(), "php", "");
foreach ($values as $value){
  var_dump(array_keys($types_arr, $value, TRUE));
}

echo "Done\n";
?>