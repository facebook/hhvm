<?hh
<<__EntryPoint>> function main(): void {
include "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
var_dump(
    $result = ldap_read($link, "o=test,$base", "(o=*)"),
    ldap_get_entries($link, $result)
);
echo "===DONE===\n";
//--remove_dummy_data($link, $base);
}
