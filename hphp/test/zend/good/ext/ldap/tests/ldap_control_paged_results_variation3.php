<?hh
<<__EntryPoint>> function main(): void {
include "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
$dn = "$base";
$filter = "(cn=user*)";
$cookie = '';
var_dump(ldap_control_paged_result($link, 2, true, $cookie));
$result = ldap_search($link, $dn, $filter, vec['cn']);
var_dump($result);
var_dump(ldap_get_entries($link, $result));
$estimated = null;
var_dump(ldap_control_paged_result_response($link, $result, inout $cookie, inout $estimated));
var_dump($estimated);
var_dump(ldap_control_paged_result($link, 20, true, $cookie));
$result = ldap_search($link, $dn, $filter, vec['cn']);
var_dump($result);
var_dump(ldap_get_entries($link, $result));
echo "===DONE===\n";
//--remove_dummy_data($link, $base);
}
