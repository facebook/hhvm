<?php

$arr = array(1, 2, 'a'=>'b');
$obj = new ArrayObject($arr, 0);

$obj->exchangeArray($obj);

$old = $obj->exchangeArray([]);
if(is_array($old)) {
  echo "is_array\n";
} else if(is_object($old)) {
  echo "is_object\n";
} else {
  echo "is neither";
}
