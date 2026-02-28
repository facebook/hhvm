<?hh

function myCallBack( $match ) :mixed{

    ZendGoodExtPcreTestsBug442142::$array[] = $match[0];
    return 'xxx';
}

abstract final class ZendGoodExtPcreTestsBug442142 {
  public static $array;
}
<<__EntryPoint>> function main(): void {
ZendGoodExtPcreTestsBug442142::$array = vec[];

$string = 'aaa bbb ccc ddd eee ccc aaa bbb';

$count = -1;
var_dump(preg_replace_callback('`a+`', myCallBack<>, $string, -1, inout $count));
var_dump(ZendGoodExtPcreTestsBug442142::$array);
}
