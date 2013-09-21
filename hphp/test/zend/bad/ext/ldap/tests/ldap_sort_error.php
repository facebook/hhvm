<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_sort($link));
var_dump(ldap_sort($link, $link));
var_dump(ldap_sort($link, $link, $link, $link));
var_dump(ldap_sort($link, $link, $link));
var_dump(ldap_sort($link, $link, "sn"));
?>
===DONE===