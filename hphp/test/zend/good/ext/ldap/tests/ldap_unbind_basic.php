<?hh
require "connect.inc";
<<__EntryPoint>> function main(): void {
$link = ldap_connect_and_bind($host, $port, $user, $passwd, $protocol_version);

var_dump(ldap_unbind($link));
echo "===DONE===\n";
}
