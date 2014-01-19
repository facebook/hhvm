<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

$dn = "dc=my-domain,dc=com";
$filter = "(cn=*)";
$cookie = '';
var_dump(
	ldap_control_paged_result($link, 2, true, $cookie),
	$result = ldap_search($link, $dn, $filter, array('cn')),
	ldap_get_entries($link, $result),
	ldap_control_paged_result_response($link, $result, $cookie),
	ldap_control_paged_result($link, 20, true, $cookie),
	$result = ldap_search($link, $dn, $filter, array('cn')),
	ldap_get_entries($link, $result)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>