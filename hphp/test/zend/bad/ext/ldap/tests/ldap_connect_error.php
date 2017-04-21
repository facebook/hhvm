<?php
require "connect.inc";

// too many arguments
var_dump(ldap_connect(null, null, null));
var_dump(ldap_connect("ldap://$host:$port/$base"));

$links = array();
$links[0] = ldap_connect($host, $port);
$links[1] = ldap_connect($host, $port);
?>
===DONE===
