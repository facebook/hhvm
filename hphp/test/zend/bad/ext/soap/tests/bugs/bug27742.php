<?php
parse_str("wsdl", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

$x = new SoapClient(dirname(__FILE__)."/bug27742.wsdl");
echo "ok\n";
?>