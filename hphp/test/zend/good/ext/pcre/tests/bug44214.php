<?hh

function myCallBack( $match ) {
    ZendGoodExtPcreTestsBug44214::$array[] = $match;
    return 'xxx';
}

abstract final class ZendGoodExtPcreTestsBug44214 {
  public static $array;
}
<<__EntryPoint>> function main(): void {
ZendGoodExtPcreTestsBug44214::$array = varray[];

$string = 'aaa bbb ccc ddd eee ccc aaa bbb';

$count = -1;
var_dump(preg_replace_callback('`a+`', fun('myCallBack'), $string, -1, inout $count));
var_dump(ZendGoodExtPcreTestsBug44214::$array);
}
