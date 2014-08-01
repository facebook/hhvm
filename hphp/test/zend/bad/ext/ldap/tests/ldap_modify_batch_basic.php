<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

$mods = array(
	array(
		"attrib"	=> "telephoneNumber",
		"modtype"	=> LDAP_MODIFY_BATCH_ADD,
		"values"	=> array(
			"+1 555 5551717"
		)
	),
	array(
		"attrib"	=> "sn",
		"modtype"	=> LDAP_MODIFY_BATCH_REPLACE,
		"values"	=> array("Brown-Smith")
	),
	array(
		"attrib"	=> "description",
		"modtype"	=> LDAP_MODIFY_BATCH_REMOVE_ALL
	)
);

var_dump(
	ldap_modify_batch($link, "cn=userA,dc=my-domain,dc=com", $mods),
	ldap_get_entries($link, ldap_search($link, "dc=my-domain,dc=com", "(sn=Brown-Smith)"))
);
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

remove_dummy_data($link);
?>