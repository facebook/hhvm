<?hh

<<__EntryPoint>>
function main_cleanup_persistent_socket() :mixed{
$errno = null;
$errstr = null;
$sock = pfsockopen('udp://127.0.0.1', 63844, inout $errno, inout $errstr);
var_dump((int)$sock);
fwrite($sock, "1");
fclose($sock);
$sock = null;
$sock2 = pfsockopen('udp://127.0.0.1', 63844, inout $errno, inout $errstr);
var_dump((int)$sock2);
fwrite($sock2, "2");
fclose($sock2);
}
