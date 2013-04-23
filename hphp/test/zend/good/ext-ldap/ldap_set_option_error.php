<?php
require "connect.inc";

$link = ldap_connect($host, $port);
$controls = array(
	array(
		array("xid" => "1.2.752.58.10.1", "iscritical" => true),
		array("xid" => "1.2.752.58.1.10", "value" => "magic"),
	),
	array(
		array("oid" => "1.2.752.58.10.1", "iscritical" => true),
		array("oid" => "1.2.752.58.1.10", "value" => "magic"),
		"weird"
	),
	array(
	),
);

// Too few parameters
var_dump(ldap_set_option());
var_dump(ldap_set_option($link));
var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION));

// Too many parameters
var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, 3, "Additional data"));

var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, 10));

foreach ($controls as $control)
	var_dump(ldap_set_option($link, LDAP_OPT_SERVER_CONTROLS, $control));

var_dump(ldap_set_option($link, 999999, 999999));
?>
===DONE===