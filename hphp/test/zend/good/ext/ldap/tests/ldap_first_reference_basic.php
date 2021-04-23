<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect_and_bind(test_host(), test_port(), test_user(), test_passwd(), test_protocol_version());
$base = test_base();
insert_dummy_data($link, $base);
ldap_add($link, "cn=userref,$base", dict[
        "objectClass" => vec["extensibleObject", "referral"],
        "cn" => "userref",
        "ref" => "cn=userA,$base",
]);
ldap_set_option($link, LDAP_OPT_DEREF, LDAP_DEREF_NEVER);
$result = ldap_search($link, "$base", "(cn=*)");
var_dump($ref = ldap_first_reference($link, $result));
$refs = null;
ldap_parse_reference($link, $ref, inout $refs);
var_dump($refs);
echo "===DONE===\n";
// Referral can only be removed with Manage DSA IT Control
//--ldap_set_option($link, LDAP_OPT_SERVER_CONTROLS, array(array("oid" => "2.16.840.1.113730.3.4.2")));
//--ldap_delete($link, "cn=userref,$base");
//--remove_dummy_data($link, $base);
}
