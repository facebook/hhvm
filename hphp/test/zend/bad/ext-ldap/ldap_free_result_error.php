<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_free_result($link));
var_dump(ldap_free_result($link, "Additional data"));
?>
===DONE===