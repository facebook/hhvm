<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_get_attributes($link));
var_dump(ldap_get_attributes($link, $link));
?>
===DONE===