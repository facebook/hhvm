<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);
var_dump(
	ldap_rename($link, "cn=userA,dc=my-domain,dc=com", "cn=userZ", "dc=my-domain,dc=com", true)
);
$result = ldap_search($link, "dc=my-domain,dc=com", "(cn=userA)", array("cn", "sn"));
$result = ldap_search($link, "dc=my-domain,dc=com", "(cn=userZ)", array("cn", "sn"));
var_dump(ldap_get_entries($link, $result));
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
ldap_rename($link, "cn=userZ,dc=my-domain,dc=com", "cn=userA", "dc=my-domain,dc=com", true);
remove_dummy_data($link);
?>