<?php
$sock = pfsockopen('udp://127.0.0.1', '63844');
var_dump((int)$sock);
@fwrite($sock, "1");
fclose($sock);
unset($sock);
$sock2 = pfsockopen('udp://127.0.0.1', '63844');
var_dump((int)$sock2);
@fwrite($sock2, "2");
fclose($sock2);
