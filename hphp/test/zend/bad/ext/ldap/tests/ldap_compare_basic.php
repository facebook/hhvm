<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);
var_dump(
	ldap_compare($link, "cn=userA,dc=my-domain,dc=com", "sn", "testSN1"),
	ldap_compare($link, "cn=userA,dc=my-domain,dc=com", "telephoneNumber", "yy-yy-yy-yy-yy")
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>