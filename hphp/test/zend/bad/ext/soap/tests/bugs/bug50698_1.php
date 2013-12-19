<?php
new SoapClient(dirname(__FILE__) . '/bug50698_1.wsdl');
echo "ok\n";
?>