<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

// Too few parameters
var_dump(ldap_unbind());

// Too many parameters
var_dump(ldap_unbind($link, "Additional data"));

// Bad parameter
var_dump(ldap_unbind("string"));

// unbind twice
var_dump(ldap_unbind($link));
var_dump(ldap_unbind($link));
?>
===DONE===