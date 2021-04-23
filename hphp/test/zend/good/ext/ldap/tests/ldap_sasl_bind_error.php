<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect(test_host(), test_port());
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());

// Invalid parameter count
var_dump(ldap_sasl_bind());

// Invalid DN
var_dump(ldap_sasl_bind($link, "Invalid DN", test_passwd(), 'DIGEST-MD5', 'realm', test_sasl_user()));

// Invalid user
var_dump(ldap_sasl_bind($link, null, "ThisIsNotCorrecttest_passwd()", 'DIGEST-MD5', "realm", "invalid" . test_sasl_user()));

// Invalid password
var_dump(ldap_sasl_bind($link, null, "ThisIsNotCorrecttest_passwd()", 'DIGEST-MD5', "realm", test_sasl_user()));

var_dump(ldap_sasl_bind($link, null, test_passwd(), 'DIGEST-MD5', "realm", "Manager", "test"));

// Invalid DN syntax
var_dump(ldap_sasl_bind($link, "unexistingProperty=weirdValue,test_user()", test_passwd()));
echo "===DONE===\n";
}
