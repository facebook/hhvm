<?hh

function myCallBack( $match ) {

    ZendGoodExtPcreTestsBug442142::$array[] = $match[0];
    return 'xxx';
}

abstract final class ZendGoodExtPcreTestsBug442142 {
  public static $array;
}
<<__EntryPoint>> function main(): void {
ZendGoodExtPcreTestsBug442142::$array = varray[];

$string = 'aaa bbb ccc ddd eee ccc aaa bbb';

$count = -1;
var_dump(preg_replace_callback('`a+`', fun('myCallBack'), $string, -1, inout $count));
var_dump(ZendGoodExtPcreTestsBug442142::$array);
}
