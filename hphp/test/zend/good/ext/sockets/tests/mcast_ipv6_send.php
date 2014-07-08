<?php
$domain = AF_INET6;
$level = IPPROTO_IPV6;
$s = socket_create($domain, SOCK_DGRAM, SOL_UDP) or die("err");

echo "Setting IPV6_MULTICAST_TTL\n";
$r = socket_set_option($s, $level, IPV6_MULTICAST_HOPS, 9);
var_dump($r);
$r = socket_get_option($s, $level, IPV6_MULTICAST_HOPS);
var_dump($r);
echo "\n";

echo "Setting IPV6_MULTICAST_LOOP\n";
$r = socket_set_option($s, $level, IPV6_MULTICAST_LOOP, 0);
var_dump($r);
$r = socket_get_option($s, $level, IPV6_MULTICAST_LOOP);
var_dump($r);
$r = socket_set_option($s, $level, IPV6_MULTICAST_LOOP, 1);
var_dump($r);
$r = socket_get_option($s, $level, IPV6_MULTICAST_LOOP);
var_dump($r);
echo "\n";

echo "Setting IPV6_MULTICAST_IF\n";
echo "interface 0:\n";
$r = socket_set_option($s, $level, IPV6_MULTICAST_IF, 0);
var_dump($r);
$r = socket_get_option($s, $level, IPV6_MULTICAST_IF);
var_dump($r);
echo "interface 1:\n";
$r = socket_set_option($s, $level, IPV6_MULTICAST_IF, 1);
var_dump($r);
$r = socket_get_option($s, $level, IPV6_MULTICAST_IF);
var_dump($r);
echo "\n";
