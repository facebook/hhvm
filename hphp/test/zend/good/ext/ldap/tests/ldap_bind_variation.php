<?hh
require "connect.inc";
<<__EntryPoint>> function main(): void {
$link = ldap_connect($host, $port);
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);
var_dump(ldap_bind($link, $user, $passwd));
echo "===DONE===\n";
}
