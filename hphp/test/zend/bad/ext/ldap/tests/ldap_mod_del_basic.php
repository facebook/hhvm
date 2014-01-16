<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

$entry = array(
	"description" => "user A"
);

var_dump(
	ldap_mod_del($link, "cn=userA,dc=my-domain,dc=com", $entry),
	ldap_get_entries(
		$link,
		ldap_search($link, "dc=my-domain,dc=com", "(description=user A)")
	)
);
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

remove_dummy_data($link);
?>