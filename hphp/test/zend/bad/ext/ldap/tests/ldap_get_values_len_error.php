<?php
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link);
$result = ldap_search($link, "dc=my-domain,dc=com", "(objectclass=organization)");
$entry = ldap_first_entry($link, $result);

// Too few parameters
var_dump(ldap_get_values_len($link));
var_dump(ldap_get_values_len($link, $entry));
var_dump(ldap_get_values_len($link, $entry, "weirdAttribute", "Additional data"));

var_dump(ldap_get_values_len($link, $entry, "inexistentAttribute"));
?>
===DONE===
<?php
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link);
?>