<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
// Too few parameters
try {
  var_dump(ldap_mod_replace());
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_mod_replace($link));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_mod_replace($link, "$base"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try {
  var_dump(ldap_mod_replace($link, "$base", dict[], "Additional data"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// DN not found
var_dump(ldap_mod_replace($link, "dc=my-domain,$base", dict[]));

// Invalid DN
var_dump(ldap_mod_replace($link, "weirdAttribute=val", dict[]));

// Invalid attributes
var_dump(ldap_mod_replace($link, "$base", dict[0 => 'dc']));
echo "===DONE===\n";
}
