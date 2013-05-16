<?php
$string = 'aaa bbb ccc ddd eee ccc aaa bbb';

$array = array();

function myCallBack( $match ) {
    global $array;
    $array[] = $match[0];
    return 'xxx';
}

var_dump(preg_replace_callback( '`a+`', 'myCallBack', $string));
var_dump($array);
?>