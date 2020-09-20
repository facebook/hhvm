<?hh

function evil($x) {

    ZendGoodExtPcreTests007::$txt[3] = "\xFF";
    var_dump($x);
    return $x[0];
}

abstract final class ZendGoodExtPcreTests007 {
  public static $txt;
}
<<__EntryPoint>> function main(): void {
ZendGoodExtPcreTests007::$txt = "ola123";
$count = -1;
var_dump(preg_replace_callback('#.#u', fun('evil'), ZendGoodExtPcreTests007::$txt, -1, inout $count));
var_dump(ZendGoodExtPcreTests007::$txt);
var_dump(preg_last_error() == PREG_NO_ERROR);

var_dump(preg_replace_callback('#.#u', fun('evil'), ZendGoodExtPcreTests007::$txt, -1, inout $count));
var_dump(preg_last_error() == PREG_BAD_UTF8_ERROR);

echo "Done!\n";
}
