<?php
require "connect.inc";

$link = ldap_connect($host, $port);
$option = null;

$controls = array(
	array("oid" => "1.2.752.58.10.1", "iscritical" => true),
	array("oid" => "1.2.752.58.1.10", "value" => "magic"),
);

var_dump(ldap_set_option($link, LDAP_OPT_DEREF, LDAP_DEREF_ALWAYS));
ldap_get_option($link, LDAP_OPT_DEREF, $option);
var_dump(
	$option === LDAP_DEREF_ALWAYS,
	ldap_set_option($link, LDAP_OPT_SIZELIMIT, 123)
);
ldap_get_option($link, LDAP_OPT_SIZELIMIT, $option);
var_dump(
	$option,
	ldap_set_option($link, LDAP_OPT_TIMELIMIT, 33)
);
ldap_get_option($link, LDAP_OPT_TIMELIMIT, $option);
var_dump(
	$option,
	ldap_set_option($link, LDAP_OPT_NETWORK_TIMEOUT, 44)
);
ldap_get_option($link, LDAP_OPT_NETWORK_TIMEOUT, $option);
var_dump(
	$option,
	ldap_set_option($link, LDAP_OPT_REFERRALS, true)
);
ldap_get_option($link, LDAP_OPT_REFERRALS, $option);
var_dump(
	(bool) $option,
	ldap_set_option($link, LDAP_OPT_RESTART, false)
);
ldap_get_option($link, LDAP_OPT_RESTART, $option);
var_dump(
	(bool) $option,
	ldap_set_option($link, LDAP_OPT_SERVER_CONTROLS, $controls)
);
ldap_get_option($link, LDAP_OPT_SERVER_CONTROLS, $option);
var_dump(
	$option,
	ldap_set_option($link, LDAP_OPT_CLIENT_CONTROLS, $controls)
);
ldap_get_option($link, LDAP_OPT_CLIENT_CONTROLS, $option);
var_dump(
	$option,
	ldap_set_option($link, LDAP_OPT_MATCHED_DN, "dc=test,dc=com")
);
ldap_get_option($link, LDAP_OPT_MATCHED_DN, $option);
var_dump($option);
?>
===DONE===