<?php
new SoapClient(dirname(__FILE__) . '/bug50698_4.wsdl');
echo "ok\n";
?>