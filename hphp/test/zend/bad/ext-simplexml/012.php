<?php 

$xml =<<<EOF
<?xml version="1.0" encoding="ISO-8859-1" ?>
<foo/>
EOF;

$sxe = simplexml_load_string($xml);


$sxe[""] = "warning";
$sxe["attr"] = "value";

echo $sxe->asXML();

$sxe["attr"] = "new value";

echo $sxe->asXML();

$sxe[] = "error";

__HALT_COMPILER();
?>
===DONE===