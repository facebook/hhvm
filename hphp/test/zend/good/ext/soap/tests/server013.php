<?hh
function Sum($a) :mixed{
  $sum = 0;
  if (is_array($a)) {
    foreach($a as $val) {
      $sum += $val;
    }
  }
  return $sum;
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(null,dict['uri'=>"http://testuri.org"]);
$server->addFunction("Sum");

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <SOAP-ENV:Body xmlns:ns1="http://linuxsrv.home/~dmitry/soap/">
    <ns1:Sum>
      <param0 SOAP-ENC:arrayType="xsd:int[2]" xsi:type="SOAP-ENC:Array">
        <val xsi:type="xsd:int">3</val>
        <val xsi:type="xsd:int">5</val>
      </param0>
    </ns1:Sum>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;
$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";
}
