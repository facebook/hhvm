<?php
include __DIR__."/mcast_helpers.php.inc";
$addr = '::1';

echo "creating send socket\n";
$sends1 = socket_create(AF_INET6, SOCK_DGRAM, SOL_UDP) or die("err");
var_dump($sends1);
$br = socket_bind($sends1, '::', 7001) or die("err");
var_dump($br);
socket_set_nonblock($sends1) or die("Could not put in non-blocking mode");

echo "creating receive socket\n";
$s = socket_create(AF_INET6, SOCK_DGRAM, SOL_UDP) or die("err");
var_dump($s);
$br = socket_bind($s, '::0', 3000) or die("err");
var_dump($br);

socket_set_option($s, IPPROTO_IPV6, IPV6_RECVPKTINFO, 1) or die("err");
socket_set_option($s, IPPROTO_IPV6, IPV6_RECVTCLASS, 1) or die("err");

$r = socket_sendmsg($sends1, [
	"name" => [ "addr" => "::1", "port" => 3000],
	"iov" => ["test ", "thing", "\n"],
	"control" => [[
		"level" => IPPROTO_IPV6,
		"type" => IPV6_PKTINFO,
		"data" => [
			'addr' => '::1',
            'ifindex' => 1 /* we're assuming loopback is 1. Is this a safe assumption? */
		],
	]]
], 0);
var_dump($r);
checktimeout($s, 500);

$data = [
    "name" => ["family" => AF_INET6, "addr" => "::1"],
    "buffer_size" => 2000,
    "controllen" => socket_cmsg_space(IPPROTO_IPV6, IPV6_PKTINFO) +
			socket_cmsg_space(IPPROTO_IPV6, IPV6_TCLASS),
];
if (!socket_recvmsg($s, $data, 0)) die("recvmsg");
print_r($data);
