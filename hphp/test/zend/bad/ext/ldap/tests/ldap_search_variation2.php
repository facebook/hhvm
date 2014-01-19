<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

var_dump(
	$result = ldap_search($link, "dc=my-domain,dc=com", "(objectclass=person)", array('sn'), 1),
	ldap_get_entries($link, $result)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>