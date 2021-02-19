<?hh
<<__EntryPoint>>
function main_entry(): void {
  require "connect.inc";

  $link = ldap_connect(test_host(), test_port());
  $option = null;

  var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, test_protocol_version()));
  ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION, inout $option);
  var_dump($option);
  echo "===DONE===\n";
}
