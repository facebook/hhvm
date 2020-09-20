<?hh <<__EntryPoint>> function main(): void {
$errno = null;
$errstr = null;
$sock = pfsockopen('udp://127.0.0.1', 63844, inout $errno, inout $errstr);
var_dump((int)$sock);
@fwrite($sock, "1");
$sock2 = pfsockopen('udp://127.0.0.1', 63844, inout $errno, inout $errstr);
var_dump((int)$sock2);
@fwrite($sock2, "2");
fclose($sock2);
fwrite($sock, "3");
}
