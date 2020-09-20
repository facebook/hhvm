<?hh
require "connect.inc";

$link = ldap_connect($host, $port);
try { var_dump(ldap_get_attributes($link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(ldap_get_attributes($link, $link));
echo "===DONE===\n";
