<?php
include __DIR__."/mcast_helpers.php.inc";
$domain = AF_INET6;
$level = IPPROTO_IPV6;
$interface = 0;
$mcastaddr = 'ff01::114';
$sblock = "?";

echo "creating send socket\n";
$sends1 = socket_create($domain, SOCK_DGRAM, SOL_UDP) or die("err");
var_dump($sends1);

echo "creating receive socket\n";
$s = socket_create($domain, SOCK_DGRAM, SOL_UDP) or die("err");
var_dump($s);
$br = socket_bind($s, '::0', 3000) or die("err");
var_dump($br);

$so = socket_set_option($s, $level, MCAST_JOIN_GROUP, array(
	"group"	=> $mcastaddr,
	"interface" => $interface,
)) or die("err");
var_dump($so);

$r = socket_sendto($sends1, $m = "testing packet", strlen($m), 0, $mcastaddr, 3000);
var_dump($r);
checktimeout($s, 500);
$r = socket_recvfrom($s, $str, 2000, 0, $from, $fromPort);
var_dump($r, $str, $from);
$sblock = $from;

$r = socket_sendto($sends1, $m = "initial packet", strlen($m), 0, $mcastaddr, 3000);
var_dump($r);

$i = 0;
checktimeout($s, 500);
while (($str = socket_read($s, 3000, 500)) !== FALSE) {
	$i++;
	echo "$i> ", $str, "\n";

if ($i == 1) {
	echo "leaving group\n";
	$so = socket_set_option($s, $level, MCAST_LEAVE_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "ignored mcast packet", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
	$r = socket_sendto($sends1, $m = "unicast packet", strlen($m), 0, "::1", 3000);
	var_dump($r);
}
if ($i == 2) {
	echo "re-joining group\n";
	$so = socket_set_option($s, $level, MCAST_JOIN_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "mcast packet", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
}
if ($i == 3) {
	break;
}

}