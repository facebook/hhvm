<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);

// Too few parameters
var_dump(ldap_compare($link));
var_dump(ldap_compare($link, $link));
var_dump(ldap_compare($link, $link, $link));

// Too many parameters
var_dump(ldap_compare($link, $link, $link, $link, "Additional data"));

var_dump(
	ldap_compare($link, "cn=userNotAvailable,dc=my-domain,dc=com", "sn", "testSN1"),
	ldap_error($link),
	ldap_errno($link)
);
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>