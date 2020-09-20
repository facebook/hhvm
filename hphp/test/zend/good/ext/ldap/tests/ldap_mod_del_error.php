<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

// Too few parameters
var_dump(ldap_mod_del());
var_dump(ldap_mod_del($link));
var_dump(ldap_mod_del($link, "$base"));

// Too many parameters
var_dump(ldap_mod_del($link, "$base", array(), "Additional data"));

// DN not found
var_dump(ldap_mod_del($link, "dc=my-domain,$base", array()));

// Invalid DN
var_dump(ldap_mod_del($link, "weirdAttribute=val", array()));

// Invalid attributes
var_dump(ldap_mod_del($link, "$base", array('dc')));
echo "===DONE===\n";
<?hh
require "connect.inc";

$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);
