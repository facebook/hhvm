<?hh
// Non-WSDL SOAP_DOCUMENT auto-names params "param%d", exercising the unnamed-param
// path in serialize_parameter.
<<__EntryPoint>>
function main(): void {
  $client = new SoapClient(null, dict[
    'location' => 'http://127.0.0.1:1/x',
    'uri' => 'urn:test',
    'style' => SOAP_DOCUMENT,
    'use' => SOAP_LITERAL,
  ]);
  try {
    $client->__soapcall('myMethod', vec[123, 'abc']);
  } catch (Exception $_) {}
  echo "done\n";
}
