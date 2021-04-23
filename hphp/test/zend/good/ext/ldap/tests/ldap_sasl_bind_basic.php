<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect(test_host(), test_port());
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());
var_dump(ldap_sasl_bind($link, null, test_passwd(), 'DIGEST-MD5', 'realm', test_sasl_user()));
echo "===DONE===\n";
}
