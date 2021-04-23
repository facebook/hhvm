<?hh
<<__EntryPoint>> function main(): void {
require "connect.inc";
$link = ldap_connect(test_host(), test_port());
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version());

// Invalid parameter count
try {
  var_dump(ldap_start_tls());
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try {
  var_dump(ldap_start_tls($link, $link));
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
