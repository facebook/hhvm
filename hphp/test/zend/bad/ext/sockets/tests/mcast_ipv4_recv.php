<?php
include __DIR__."/mcast_helpers.php.inc";
$domain = AF_INET;
$level = IPPROTO_IP;
$interface = "lo";
$mcastaddr = '224.0.0.23';
$sblock = "127.0.0.1";

echo "creating send socket bound to 127.0.0.1\n";
$sends1 = socket_create($domain, SOCK_DGRAM, SOL_UDP);
$br = socket_bind($sends1, '127.0.0.1');
var_dump($br);

echo "creating unbound socket and hoping the routing table causes an interface other than lo to be used for sending messages to $mcastaddr\n";
$sends2 = socket_create($domain, SOCK_DGRAM, SOL_UDP);
var_dump($br);

echo "creating receive socket\n";
$s = socket_create($domain, SOCK_DGRAM, SOL_UDP);
var_dump($s);
$br = socket_bind($s, '0.0.0.0', 3000);
var_dump($br);

$so = socket_set_option($s, $level, MCAST_JOIN_GROUP, array(
	"group"	=> $mcastaddr,
	"interface" => $interface,
));
var_dump($so);

$r = socket_sendto($sends1, $m = "initial packet", strlen($m), 0, $mcastaddr, 3000);
var_dump($r);

$i = 0;
checktimeout($s, 500);
while (($str = socket_read($s, 3000)) !== FALSE) {
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
	$r = socket_sendto($sends1, $m = "unicast packet", strlen($m), 0, "127.0.0.1", 3000);
	var_dump($r);
}
if ($i == 2) {
	echo "re-joining group\n";
	$so = socket_set_option($s, $level, MCAST_JOIN_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
	));
	var_dump($so);
	$r = socket_sendto($sends2, $m = "ignored mcast packet (different interface)", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
	$r = socket_sendto($sends1, $m = "mcast packet", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
}
if ($i == 3) {
	echo "blocking source\n";
	$so = socket_set_option($s, $level, MCAST_BLOCK_SOURCE, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
		"source" => $sblock,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "ignored packet (blocked source)", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
	$r = socket_sendto($sends1, $m = "unicast packet", strlen($m), 0, "127.0.0.1", 3000);
	var_dump($r);
}
if ($i == 4) {
	echo "unblocking source\n";
	$so = socket_set_option($s, $level, MCAST_UNBLOCK_SOURCE, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
		"source" => $sblock,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "mcast packet from 127.0.0.1", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
}
if ($i == 5) {
	echo "leaving group\n";
	$so = socket_set_option($s, $level, MCAST_LEAVE_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "ignored mcast packet", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
	$r = socket_sendto($sends1, $m = "unicast packet", strlen($m), 0, "127.0.0.1", 3000);
	var_dump($r);
}
if ($i == 6) {
	echo "joining source group\n";
	$so = socket_set_option($s, $level, MCAST_JOIN_SOURCE_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
		"source" => $sblock,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "mcast packet from 127.0.0.1", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
}
if ($i == 7) {
	echo "leaving source group\n";
	$so = socket_set_option($s, $level, MCAST_LEAVE_SOURCE_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
		"source" => $sblock,
	));
	var_dump($so);
	$r = socket_sendto($sends1, $m = "ignored mcast packet", strlen($m), 0, $mcastaddr, 3000);
	var_dump($r);
	$r = socket_sendto($sends1, $m = "unicast packet", strlen($m), 0, "127.0.0.1", 3000);
	var_dump($r);
}
if ($i == 8) {
/*	echo "rjsg\n";
	$so = socket_set_option($s, $level, MCAST_JOIN_GROUP, array(
		"group"	=> $mcastaddr,
		"interface" => $interface,
	));
	var_dump($so);*/
	break;
}

}