<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_get_dn($link));
var_dump(ldap_get_dn($link, $link));
?>
===DONE===