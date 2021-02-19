<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect(test_host(), test_port());
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());

// Invalid parameter count
var_dump(ldap_start_tls());
var_dump(ldap_start_tls($link, $link));
echo "===DONE===\n";
}
