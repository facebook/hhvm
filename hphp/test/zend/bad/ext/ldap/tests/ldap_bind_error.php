<?php
require "connect.inc";

$link = ldap_connect($host, $port);
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);

// Invalid parameter count
var_dump(ldap_bind($link, $user, $passwd, null));

// Invalid password
var_dump(ldap_bind($link, $user, "ThisIsNotCorrect$passwd"));

// Invalid DN syntax
var_dump(ldap_bind($link, "unexistingProperty=weirdValue,$user", $passwd));
?>
===DONE===