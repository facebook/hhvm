<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
// Too few parameters
var_dump(ldap_compare($link));
var_dump(ldap_compare($link, $link));
var_dump(ldap_compare($link, $link, $link));

// Too many parameters
var_dump(ldap_compare($link, $link, $link, $link, "Additional data"));

var_dump(
    ldap_compare($link, "cn=userNotAvailable,$base", "sn", "testSN1"),
    ldap_error($link),
    ldap_errno($link)
);
echo "===DONE===\n";
//--remove_dummy_data($link, $base);
}
