<?php
class PublisherService {
  function add($publisher) {
    return $publisher->region_id;
  }
}
$input =
'<?xml version="1.0" encoding="UTF-8"?>
<soapenv:Envelope
xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
xmlns:xsd="http://www.w3.org/2001/XMLSchema"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <soapenv:Body>
    <ns1:add xmlns:ns1="urn:PublisherService" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
      <publisher href="#id0"/>
    </ns1:add>
    <multiRef xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"
xmlns:ns3="http://soap.dev/soap/types" id="id0" soapenc:root="0"
soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
xsi:type="ns3:publisher">
      <region_id href="#id5"/>
    </multiRef>
    <multiRef xmlns:ns5="http://soap.dev/soap/types"
xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" id="id5"
soapenc:root="0"
soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
xsi:type="xsd:long">9</multiRef>
  </soapenv:Body>
</soapenv:Envelope>';
ini_set('soap.wsdl_cache_enabled', false);
$server = new SoapServer(dirname(__FILE__)."/bug36908.wsdl");
$server->setClass("PublisherService");
$server->handle($input);
?>