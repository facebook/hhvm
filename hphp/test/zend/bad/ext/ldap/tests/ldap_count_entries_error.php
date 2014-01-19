<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_count_entries($link));
var_dump(ldap_count_entries($link, $link));
?>
===DONE===