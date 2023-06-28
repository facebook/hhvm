<?hh

<<__EntryPoint>>
function main_ftell() :mixed{
$errno = null;
$errstr = null;
$s = fsockopen("udp://127.0.0.1", 12345, inout $errno, inout $errstr);
var_dump(fwrite($s, "foomeme"));
var_dump(ftell($s));
var_dump(fseek($s, 7, SEEK_CUR));
var_dump(fseek($s, 3));
}
