<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_rename($link));
var_dump(ldap_rename($link, "cn=userNotFound,dc=my-domain,dc=com", "cn=userZ", "dc=my-domain,dc=com", true));
?>
===DONE===