<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_rename($link));
var_dump(ldap_rename($link, "cn=userNotFound,$base", "cn=userZ", "$base", true));
?>
===DONE===
