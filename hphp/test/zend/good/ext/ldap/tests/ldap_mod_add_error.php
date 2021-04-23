<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
// Too few parameters
try {
  var_dump(ldap_mod_add());
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_mod_add($link));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_mod_add($link, "$base"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try {
  var_dump(ldap_mod_add($link, "$base", dict[], "Additional data"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
// DN not found
var_dump(ldap_mod_add($link, "dc=my-domain,$base", dict[]));

// Invalid DN
var_dump(ldap_mod_add($link, "weirdAttribute=val", dict[]));

$entry = dict[
    "objectClass"   => vec[
        "top",
        "dcObject",
        "organization"],
    "dc"            => "my-domain",
    "o"             => "my-domain",
];

ldap_add($link, "dc=my-domain,$base", $entry);

$entry2 = $entry;
$entry2["dc"] = "Wrong Domain";

var_dump(ldap_mod_add($link, "dc=my-domain,$base", $entry2));

$entry2 = $entry;
$entry2["weirdAttribute"] = "weirdVal";

var_dump(ldap_mod_add($link, "dc=my-domain,$base", $entry2));
echo "===DONE===\n";
//--ldap_delete($link, "dc=my-domain,$base");
}
