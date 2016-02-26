<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);
var_dump(
	ldap_compare($link, "cn=userA,$base", "sn", "testSN1"),
	ldap_compare($link, "cn=userA,$base", "telephoneNumber", "yy-yy-yy-yy-yy")
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link, $base);
?>
