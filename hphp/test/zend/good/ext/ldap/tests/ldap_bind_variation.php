<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect(test_host(), test_port());
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());
var_dump(ldap_bind($link, test_user(), test_passwd()));
echo "===DONE===\n";
}
