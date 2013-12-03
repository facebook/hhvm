<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

$dn = "dc=my-domain,dc=com";
$filter = "(objectclass=person)";
var_dump(
	$result = ldap_search($link, $dn, $filter, array('sn'), 1, 1, 3, LDAP_DEREF_SEARCHING),
	ldap_get_entries($link, $result)
);
var_dump(
	$result = ldap_search($link, $dn, $filter, array('sn'), 1, 1, 3, LDAP_DEREF_FINDING),
	ldap_get_entries($link, $result)
);
var_dump(
	$result = ldap_search($link, $dn, $filter, array('sn'), 1, 1, 3, LDAP_DEREF_ALWAYS),
	ldap_get_entries($link, $result)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>