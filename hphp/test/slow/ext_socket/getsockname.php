<?hh


<<__EntryPoint>>
function main_getsockname() {
$s = socket_create(AF_UNIX, SOCK_STREAM, 0);
var_dump($s);

$f = sys_get_temp_dir().'/socktest'.rand();
$ret = socket_bind($s, $f);
var_dump($ret);

$n = null;
$port = null;
$ret = socket_getsockname($s, inout $n, inout $port);
var_dump($ret);

var_dump($f);
var_dump($n);
var_dump($f === $n);

socket_close($s);
unlink($f);
}
