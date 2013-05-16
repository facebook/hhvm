<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_first_attribute($link));
var_dump(ldap_first_attribute($link, $link));
?>
===DONE===