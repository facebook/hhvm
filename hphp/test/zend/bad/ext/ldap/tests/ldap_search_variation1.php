<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

$dn = "dc=my-domain,dc=com";
$filter = "(dc=*)";
var_dump(
	$result = ldap_search($link, "dc=my-domain,dc=com", "(dc=*)", array('dc')),
	ldap_get_entries($link, $result)
);
?>
===DONE===?>
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>