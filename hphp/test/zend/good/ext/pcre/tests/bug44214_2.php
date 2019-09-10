<?hh
$string = 'aaa bbb ccc ddd eee ccc aaa bbb';

ZendGoodExtPcreTestsBug442142::$array = array();

function myCallBack( $match ) {

    ZendGoodExtPcreTestsBug442142::$array[] = $match[0];
    return 'xxx';
}

$count = -1;
var_dump(preg_replace_callback('`a+`', 'myCallBack', $string, -1, inout $count));
var_dump(ZendGoodExtPcreTestsBug442142::$array);

abstract final class ZendGoodExtPcreTestsBug442142 {
  public static $array;
}
