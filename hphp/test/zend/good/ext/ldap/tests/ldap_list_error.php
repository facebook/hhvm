<?php
include "connect.inc";

$link = ldap_connect($host, $port);

// Too few parameters
var_dump(ldap_list());
var_dump(ldap_list($link));
var_dump(ldap_list($link, $link));

// Too many parameters
var_dump(ldap_list($link, "$base", "(objectClass=*)", array(), 0, 0, 0, 0 , "Additional data"));
?>
===DONE===
