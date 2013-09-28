<?php
/* Prototype  : proto string gethostbyaddr(string ip_address)
 * Description: Get the Internet host name corresponding to a given IP address 
 * Source code: ext/standard/dns.c
 * Alias to functions: 
 */


echo "Testing gethostbyaddr : error conditions\n";

// Zero arguments
echo "\n-- Testing gethostbyaddr function with Zero arguments --\n";
var_dump( gethostbyaddr() );

//Test gethostbyaddr with one more than the expected number of arguments
echo "\n-- Testing gethostbyaddr function with more than expected no. of arguments --\n";
$ip_address = 'string_val';
$extra_arg = 10;
var_dump( gethostbyaddr($ip_address, $extra_arg) );

echo "\n-- Testing gethostbyaddr function with invalid addresses --\n";

$ip_address = 'invalid';
var_dump( gethostbyaddr($ip_address) );

$ip_address = '300.1.2.3';
var_dump( gethostbyaddr($ip_address) );

$ip_address = '256.1.2.3';
var_dump( gethostbyaddr($ip_address) );

echo "Done";
?>