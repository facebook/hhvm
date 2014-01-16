<?php
try {
    new SoapClient(dirname(__FILE__) . '/bug50698_3.wsdl');
    echo "Call: \"new SoapClient(dirname(__FILE__).'/bug50698_3.wsdl');\" should throw an exception of type 'SoapFault'";
} catch (SoapFault $e) {
    if ($e->faultcode == 'WSDL' && $e->faultstring == 'SOAP-ERROR: Parsing WSDL: Could not find any usable binding services in WSDL.') {
        echo "ok\n";
    } else {
        echo "Call: \"new SoapClient(dirname(__FILE__).'/bug50698_3.wsdl');\" threw a SoapFault with an incorrect faultcode or faultmessage.";
        print_r($e);
    }    
}
?>