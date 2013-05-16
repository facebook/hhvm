<?php
require "connect.inc";

$link = ldap_connect($host, $port);
$option = null;

// Too few parameters
var_dump(ldap_get_option());
var_dump(ldap_get_option($link));
var_dump(ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION));

// Too many parameters
var_dump(
	ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION, $option, "Additional data"),
	$option
);
?>
===DONE===