<?php
$string = 'aaa bbb ccc ddd eee ccc aaa bbb';

ZendGoodExtPcreTestsBug44214::$array = array();

function myCallBack( $match ) {

    ZendGoodExtPcreTestsBug44214::$array[] = $match;
    return 'xxx';
}

var_dump(preg_replace_callback( '`a+`', 'myCallBack', $string));
var_dump(ZendGoodExtPcreTestsBug44214::$array);

abstract final class ZendGoodExtPcreTestsBug44214 {
  public static $array;
}
