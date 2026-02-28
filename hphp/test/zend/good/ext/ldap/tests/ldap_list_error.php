<?hh
<<__EntryPoint>> function main(): void {
include "connect.inc";
$link = ldap_connect(test_host(), test_port());
$base = test_base();
// Too few parameters
try { var_dump(ldap_list()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_list($link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_list($link, $link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try { var_dump(ldap_list($link, "$base", "(objectClass=*)", vec[], 0, 0, 0, 0 , "Additional data")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
