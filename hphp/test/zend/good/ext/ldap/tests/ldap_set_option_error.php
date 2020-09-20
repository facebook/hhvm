<?hh
require "connect.inc";

$link = ldap_connect($host, $port);
$controls = varray[
	varray[
		darray["xid" => "1.2.752.58.10.1", "iscritical" => true],
		darray["xid" => "1.2.752.58.1.10", "value" => "magic"],
	],
	varray[
		darray["oid" => "1.2.752.58.10.1", "iscritical" => true],
		darray["oid" => "1.2.752.58.1.10", "value" => "magic"],
		"weird"
	],
	varray[
	],
];

// Too few parameters
try { var_dump(ldap_set_option()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_set_option($link)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Too many parameters
try { var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, 3, "Additional data")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, 10));

foreach ($controls as $control)
	var_dump(ldap_set_option($link, LDAP_OPT_SERVER_CONTROLS, $control));

var_dump(ldap_set_option($link, 999999, 999999));
echo "===DONE===\n";
