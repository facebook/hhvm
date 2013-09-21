<?php
include "connect.inc";

$link = ldap_connect($host, $port);

// Too few parameters
var_dump(ldap_read());
var_dump(ldap_read($link));
var_dump(ldap_read($link, $link));

// Too many parameters
var_dump(ldap_read($link, "dc=my-domain,dc=com", "(objectClass=*)", array(), 0, 0, 0, 0 , "Additional data"));
?>
===DONE===