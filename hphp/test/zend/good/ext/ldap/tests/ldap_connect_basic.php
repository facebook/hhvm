<?hh
<<__EntryPoint>>
function main_entry(): void {
  require "connect.inc";

  $link = ldap_connect(test_host(), test_port());
  var_dump($link);
  echo "===DONE===\n";
}
