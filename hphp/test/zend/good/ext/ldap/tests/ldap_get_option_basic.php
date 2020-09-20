<?hh
<<__EntryPoint>>
function main_entry(): void {
  require "connect.inc";

  $link = ldap_connect($host, $port);
  $option = null;
  ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);

  var_dump(
  	ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION, inout $option),
  	$option
  );
  echo "===DONE===\n";
}
