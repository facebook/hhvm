<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

$addGivenName = array(
	array(
		"attrib"	=> "givenName",
		"modtype"	=> LDAP_MODIFY_BATCH_ADD,
		"values"	=> array("Jack")
	)
);

// Too few parameters
var_dump(ldap_modify_batch());
var_dump(ldap_modify_batch($link));
var_dump(ldap_modify_batch($link, "$base"));

// Too many parameters
var_dump(ldap_modify_batch($link, "$base", $addGivenName, "Invalid additional parameter"));

// DN not found
var_dump(ldap_modify_batch($link, "cn=not-found,$base", $addGivenName));

// Invalid DN
var_dump(ldap_modify_batch($link, "weirdAttribute=val", $addGivenName));

// prepare
$entry = array(
	"objectClass"	=> array(
		"top",
		"dcObject",
		"organization"),
	"dc"			=> "my-domain",
	"o"				=> "my-domain",
);

ldap_add($link, "dc=my-domain,$base", $entry);

// invalid domain
$mods = array(
	array(
		"attrib"	=> "dc",
		"modtype"	=> LDAP_MODIFY_BATCH_REPLACE,
		"values"	=> array("Wrong Domain")
	)
);

var_dump(ldap_modify_batch($link, "dc=my-domain,$base", $mods));

// invalid attribute
$mods = array(
	array(
		"attrib"	=> "weirdAttribute",
		"modtype"	=> LDAP_MODIFY_BATCH_ADD,
		"values"	=> array("weirdVal", "anotherWeirdval")
	)
);

var_dump(ldap_modify_batch($link, "dc=my-domain,$base", $mods));
echo "===DONE===\n";
<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

ldap_delete($link, "dc=my-domain,$base");
