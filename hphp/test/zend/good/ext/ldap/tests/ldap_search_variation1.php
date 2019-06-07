<?hh
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);

$dn = "$base";
$filter = "(dc=*)";
var_dump(
	$result = ldap_search($link, "o=test,$base", "(o=*)", array('o')),
	ldap_get_entries($link, $result)
);
echo "===DONE===\n";
<?hh
include "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
remove_dummy_data($link, $base);
