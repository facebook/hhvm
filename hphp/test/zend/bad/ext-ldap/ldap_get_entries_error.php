<?php
require "connect.inc";

$link = ldap_connect($host, $port);

// Too few parameters
var_dump(ldap_get_entries($link));

// Too many parameters
var_dump(ldap_get_entries($link, $link, "Additional data"));

// Bad parameter
var_dump(ldap_get_entries($link, "string"));
?>
===DONE===