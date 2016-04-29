<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);

$entry = array(
	"description"	=> "Domain description",
);

var_dump(
	ldap_mod_add($link, "o=test,$base", $entry),
	ldap_get_entries(
		$link,
		ldap_search($link, "o=test,$base", "(Description=Domain description)")
	)
);
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

remove_dummy_data($link, $base);
?>
