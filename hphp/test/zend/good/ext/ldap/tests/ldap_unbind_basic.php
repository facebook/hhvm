<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

var_dump(ldap_unbind($link));
?>
===DONE===