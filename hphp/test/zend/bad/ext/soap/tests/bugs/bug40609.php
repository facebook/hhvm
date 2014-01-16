<?php
ini_set("soap.wsdl_cache_enabled", 0);

$c = new SoapClient(dirname(__FILE__)."/bug40609.wsdl", array('trace' => 1, 'exceptions' => 0));

$c->update(array('symbol' => new SoapVar("<symbol>MSFT</symbol>", XSD_ANYXML),
                 'price' =>  new SoapVar("<price>1000</price>", XSD_ANYXML)));
echo $c->__getLastRequest();
echo "ok\n";
?>