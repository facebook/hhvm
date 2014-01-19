<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_next_reference($link));
var_dump(ldap_next_reference($link, $link, "Additional data"));
var_dump(ldap_next_reference($link, $link));
?>
===DONE===