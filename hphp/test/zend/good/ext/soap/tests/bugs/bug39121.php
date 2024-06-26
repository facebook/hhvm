<?hh
class LocalSoapClient extends SoapClient {
  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
    return <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" soap:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
        <soap:Body>
                <getDIDAreaResponse xmlns="http://didx.org/GetList">
                        <soapenc:Array soapenc:arrayType="xsd:string[2]" xsi:type="soapenc:Array">
                                <item xsi:type="xsd:string">StateCode</item>
                                <item xsi:type="xsd:string">description</item>
                        </soapenc:Array>
                        <soapenc:Array soapenc:arrayType="xsd:anyType[2]" xsi:type="soapenc:Array">
                                <item xsi:type="xsd:int">241</item>
                                <item xsi:type="xsd:string">Carabobo</item>
                        </soapenc:Array>
                        <soapenc:Array soapenc:arrayType="xsd:anyType[2]" xsi:type="soapenc:Array">
                                <item xsi:type="xsd:int">243</item>
                                <item xsi:type="xsd:string">Aragua and Carabobo</item>
                        </soapenc:Array>
                        <soapenc:Array soapenc:arrayType="xsd:anyType[2]" xsi:type="soapenc:Array">
                                <item xsi:type="xsd:int">261</item>
                                <item xsi:type="xsd:string">Zulia</item>
                        </soapenc:Array>
                </getDIDAreaResponse>
        </soap:Body>
</soap:Envelope>
EOF;
  }
}
<<__EntryPoint>>
function main_entry(): void {

  $client = new LocalSoapClient(NULL, dict['location'=>'test://','uri'=>'test://']);
  print_r($client->__soapcall('getDIDAreaResponse', vec[]));
}
