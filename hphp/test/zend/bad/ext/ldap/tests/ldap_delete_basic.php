<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
ldap_add($link, "dc=my-domain,dc=com", array(
	"objectClass"	=> array(
		"top",
		"dcObject",
		"organization"),
	"dc"			=> "my-domain",
	"o"				=> "my-domain",
));

var_dump(
	ldap_delete($link, "dc=my-domain,dc=com"),
	@ldap_search($link, "dc=my-domain,dc=com", "(o=my-domain)")
);
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

ldap_delete($link, "dc=my-domain,dc=com");
?>