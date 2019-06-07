<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);
$result = ldap_search($link, "$base", "(cn=userC)");
$entry = ldap_first_entry($link, $result);
$attribute = ldap_first_attribute($link, $entry);
var_dump(
	ldap_next_attribute($link, $entry),
	ldap_next_attribute($link, $entry),
	ldap_next_attribute($link, $entry),
	ldap_next_attribute($link, $entry)
);
echo "===DONE===\n";
<?hh
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link, $base);
