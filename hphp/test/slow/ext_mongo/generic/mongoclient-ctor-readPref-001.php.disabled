<?php
require_once __DIR__."/../utils/server.inc";
$host = standalone_hostname();
$port = standalone_port();
$db   = dbname();

$baseString = sprintf("mongodb://%s:%d/%s?readPreference=", $host, $port, $db);

$a = array(
	'primary', 'PrIMary',
	'primaryPreferred',	'primarypreferred', 'PRIMARYPREFERRED',
	'secondary', 'SECONdary',
	'secondaryPreferred', 'SecondaryPreferred',
	'NEAREST', 'nearesT',
	'nonsense'
);

foreach ($a as $value) {
	$m = new mongo($baseString . $value);
	$rp = $m->getReadPreference();
	echo $rp["type"], "\n";
}
?>