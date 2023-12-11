<?hh
class PHPObject {
}

function test() :mixed{
	return new PHPObject();
}
<<__EntryPoint>>
function entrypoint_bug42183(): void {

  $req = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://ws.sit.com" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:test/></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;

  $server = new SoapServer(NULL, dict['uri' => 'http://ws.sit.com', 
  	'classmap' => dict['Object' => 'PHPObject']]);
  $server->addFunction("test");
  ob_start();
  $server->handle($req);
  ob_end_clean();
  echo "ok\n";
}
