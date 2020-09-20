<?hh
require "connect.inc";

$link = ldap_connect($host, $port);
try { var_dump(ldap_next_entry($link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_next_entry($link, $link, "Additional data")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(ldap_next_entry($link, $link));
echo "===DONE===\n";
