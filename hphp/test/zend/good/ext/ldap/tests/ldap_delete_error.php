<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
// Too few parameters
try {
  var_dump(ldap_delete());
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_delete($link));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try {
  var_dump(ldap_delete($link, "$base", "Additional data"));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Invalid DN
var_dump(
    ldap_delete($link, "weirdAttribute=val"),
    ldap_error($link),
    ldap_errno($link)
);

// Deleting unexisting data
var_dump(
    ldap_delete($link, "dc=my-domain,$base"),
    ldap_error($link),
    ldap_errno($link)
);
echo "===DONE===\n";
}
