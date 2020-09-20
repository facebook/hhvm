<?hh
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump(ldap_free_result($link));
try { var_dump(ldap_free_result($link, "Additional data")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
