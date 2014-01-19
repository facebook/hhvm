<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

// Too few parameters
var_dump(ldap_delete());
var_dump(ldap_delete($link));

// Too many parameters
var_dump(ldap_delete($link, "dc=my-domain,dc=com", "Additional data"));

// Invalid DN
var_dump(
	ldap_delete($link, "weirdAttribute=val"),
	ldap_error($link),
	ldap_errno($link)
);

// Deleting unexisting data
var_dump(
	ldap_delete($link, "dc=my-domain,dc=com"),
	ldap_error($link),
	ldap_errno($link)
);
?>
===DONE===
<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
?>