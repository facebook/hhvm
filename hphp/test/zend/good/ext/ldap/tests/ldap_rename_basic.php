<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
var_dump(
    ldap_rename($link, "cn=userA,$base", "cn=userZ", "$base", true)
);
$result = ldap_search($link, "$base", "(cn=userA)", array("cn", "sn"));
$result = ldap_search($link, "$base", "(cn=userZ)", array("cn", "sn"));
var_dump(ldap_get_entries($link, $result));
echo "===DONE===\n";
//--ldap_rename($link, "cn=userZ,$base", "cn=userA", "$base", true);
//--remove_dummy_data($link, $base);
}
