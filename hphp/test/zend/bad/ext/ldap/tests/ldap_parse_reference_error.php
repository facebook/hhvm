<?php
require "connect.inc";
$link = ldap_connect($host, $port);
$refs = null;
var_dump(
	ldap_parse_reference($link, $link),
	ldap_parse_reference($link, $link, $refs),
	ldap_parse_reference($link, $refs, $refs, "Additional data"),
	$refs
);
?>
===DONE===