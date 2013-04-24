<?php

$array = array(
	"127.0.0.1",
	"10.0.0.1",
	"255.255.255.255",
	"255.255.255.0",
	"0.0.0.0",
	"66.163.161.116",
);

foreach ($array as $ip) {
	var_dump($long = ip2long($ip));
	var_dump(long2ip($long));
}

var_dump(ip2long());
var_dump(ip2long(""));
var_dump(ip2long("777.777.777.777"));
var_dump(ip2long("111.111.111.111"));
var_dump(ip2long(array()));

var_dump(long2ip());
var_dump(long2ip(-110000));
var_dump(long2ip(""));
var_dump(long2ip(array()));

echo "Done\n";
?>