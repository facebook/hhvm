<?php
/* Prototype  : int ip2long(string ip_address)
 * Description: Converts a string containing an (IPv4) Internet Protocol dotted address into a proper address 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */

$ips = array(
	"1.1.011.011",
	"127.0.0.1",
	"1.1.071.071",
	"0.0.0.0",
	"1.1.081.081",
	"192.168.0.0",
	"256.0.0.1",
	"192.168.0xa.5",
);

foreach($ips as $ip) {
	var_dump(ip2long($ip));
}

?>
===DONE===