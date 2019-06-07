<?hh
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);

$dn = "$base";
$filter = "(cn=user*)";
var_dump(
	ldap_control_paged_result($link, 1),
	$result = ldap_search($link, $dn, $filter, array('cn')),
	ldap_get_entries($link, $result)
);
echo "===DONE===\n";
<?hh
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link, $base);
