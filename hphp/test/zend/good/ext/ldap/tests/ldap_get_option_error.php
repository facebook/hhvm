<?hh
require "connect.inc";

$link = ldap_connect($host, $port);
$option = null;

// Too few parameters
try { var_dump(ldap_get_option()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_get_option($link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try {
	var_dump(
		ldap_get_option($link, LDAP_OPT_PROTOCOL_VERSION, inout $option, "Additional data"),
		$option
	);
} catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
