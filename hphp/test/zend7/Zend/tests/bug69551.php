<?php
$ini = <<<INI
[Network.eth0]
SubnetMask = "
"
INI;
$settings = parse_ini_string($ini, false, INI_SCANNER_RAW);
var_dump($settings);
?>
