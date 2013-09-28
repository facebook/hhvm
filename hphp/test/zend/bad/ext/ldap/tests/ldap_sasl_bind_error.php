<?php
require "connect.inc";

$link = ldap_connect($host, $port);
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);

// Invalid parameter count
var_dump(ldap_sasl_bind());

// Invalid DN
var_dump(ldap_sasl_bind($link, "Invalid DN", $passwd, 'DIGEST-MD5', 'realm', $sasl_user));

// Invalid user
var_dump(ldap_sasl_bind($link, null, "ThisIsNotCorrect$passwd", 'DIGEST-MD5', "realm", "invalid$sasl_user"));

// Invalid password
var_dump(ldap_sasl_bind($link, null, "ThisIsNotCorrect$passwd", 'DIGEST-MD5', "realm", $sasl_user));

var_dump(ldap_sasl_bind($link, null, $passwd, 'DIGEST-MD5', "realm", "Manager", "test"));

// Invalid DN syntax
var_dump(ldap_sasl_bind($link, "unexistingProperty=weirdValue,$user", $passwd));
?>
===DONE===