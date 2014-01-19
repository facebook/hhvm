<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);
$result = ldap_list($link, "dc=my-domain,dc=com", "(objectClass=person)");
$entry = ldap_first_entry($link, $result);
var_dump(
	$entry = ldap_next_entry($link, $entry),
	ldap_get_values($link, $entry, 'sn'),
	$entry = ldap_next_entry($link, $entry)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>