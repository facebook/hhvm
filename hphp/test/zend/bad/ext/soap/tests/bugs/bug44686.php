<?php
new SoapClient(dirname(__FILE__) . "/bug44686.wsdl");
echo "ok\n";
?>