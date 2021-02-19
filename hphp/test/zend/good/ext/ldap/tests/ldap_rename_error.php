<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect(test_host(), test_port());
try { var_dump(ldap_rename($link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$base = test_base();
var_dump(ldap_rename($link, "cn=userNotFound,$base", "cn=userZ", "$base", true));
echo "===DONE===\n";
}
