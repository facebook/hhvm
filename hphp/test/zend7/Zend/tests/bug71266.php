<?php
$one = 1;
$two = 2;
$arr = ['foo' => $one, 'bar' => $two];
$obj = (object) $arr;
foreach ($obj as $val) {
    var_dump($val);
    $obj->bar = 42;
}

$arr = ['foo' => $one, 'bar' => $two];
$obj = (object) $arr;
next($obj);
var_dump(current($arr));
?>
