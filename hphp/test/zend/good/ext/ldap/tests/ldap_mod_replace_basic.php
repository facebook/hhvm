<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
insert_dummy_data($link, $base);

$entry = array(
	"description" => "user X"
);

var_dump(
	ldap_mod_replace($link, "cn=userA,$base", $entry),
	ldap_get_entries(
		$link,
		ldap_search($link, "$base", "(description=user X)", array("description"))
	)
);
echo "===DONE===\n";
<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

remove_dummy_data($link, $base);
