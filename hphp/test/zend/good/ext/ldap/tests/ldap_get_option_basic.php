<?php
require "connect.inc";

$link = ldap_connect($host, $port);
$option = null;
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);

var_dump(
	ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION, $option),
	$option
);
?>
===DONE===