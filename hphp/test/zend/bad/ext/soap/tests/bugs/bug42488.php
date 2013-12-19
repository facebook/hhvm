<?php
$request = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="test:\" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:getBadUTF/></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;
$soap = new SoapServer(NULL, array('uri'=>'test://'));
function getBadUTF(){
    return "stuff\x93thing";
}
$soap->addFunction('getBadUTF');
$soap->handle($request);
?>