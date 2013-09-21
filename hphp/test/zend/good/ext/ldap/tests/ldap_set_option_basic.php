<?php
require "connect.inc";

$link = ldap_connect($host, $port);
$option = null;

var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version));
ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION, $option);
var_dump($option);
?>
===DONE===