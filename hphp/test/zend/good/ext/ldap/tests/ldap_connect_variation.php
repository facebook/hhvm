<?hh
<<__EntryPoint>>
function main_entry(): void {
  require "connect.inc";

  // no hostname, no port
  $link = ldap_connect();
  var_dump($link);

  // no port
  $link = ldap_connect($host);
  var_dump($link);

  // URI
  $link = ldap_connect("ldap://$host:$port");
  var_dump($link);

  // URI no port
  $link = ldap_connect("ldap://$host");
  var_dump($link);

  // bad hostname (connect should work, not bind)
  $link = ldap_connect("nonexistent" . $host);
  var_dump($link);
  echo "===DONE===\n";
}
