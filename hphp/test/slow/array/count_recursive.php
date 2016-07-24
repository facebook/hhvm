<?hh

$arr = [];
$arr[] = &$arr;
var_dump(count($arr, COUNT_RECURSIVE));
