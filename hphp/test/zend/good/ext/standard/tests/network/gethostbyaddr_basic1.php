<?php
/* Prototype  : string gethostbyaddr  ( string $ip_address  )
 * Description: Get the Internet host name corresponding to a given IP address 
 * Source code: ext/standard/dns.c
*/

echo "*** Testing gethostbyaddr() : basic functionality ***\n";
echo gethostbyaddr("127.0.0.1")."\n";

?>
===DONE===
