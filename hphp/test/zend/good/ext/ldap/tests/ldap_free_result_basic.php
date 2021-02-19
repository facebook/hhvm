<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
$result = ldap_search($link, "$base", "(objectclass=person)");
var_dump(ldap_free_result($result));
echo "===DONE===\n";
//--remove_dummy_data($link, $base);
}
