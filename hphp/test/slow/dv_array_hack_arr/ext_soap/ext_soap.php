<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) :mixed{ VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

// TODO(#2512714): if you comment out the function hello and you'll SEGV :)
function hello() :mixed{
  return 'Hello World';
}

function add($a, $b) :mixed{
  return (int)$a + (int)$b;
}
function sub($a, $b) :mixed{
  return (int)$a - (int)$b;
}
function sum($a) :mixed{
  $sum = 0;
  foreach ($a as $v) {
    $sum += (int)$v;
  }
  return $sum;
}
function fault() :mixed{
 return new SoapFault('MyFault', 'My fault string');
}

//////////////////////////////////////////////////////////////////////

function verify_response($req, $expected) :mixed{


  ob_start();
  DvArrayHackArrExtSoapExtSoapPhp::$server->handle($req);
  $res = ob_get_contents();
  ob_end_clean();
  VS($res, $expected);
}

function VSOAPEX($request, $response) :mixed{
  verify_response
      ("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>".
       "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=".
       "\"http://schemas.xmlsoap.org/soap/encoding/\"".
       " xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"".
       " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"".
       " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"".
       " xmlns:si=\"http://soapinterop.org/xsd\">".
       "<SOAP-ENV:Body>".
       $request.
       "  </SOAP-ENV:Body>".
       "</SOAP-ENV:Envelope>",
       $response
       )
  ;
}

function VSOAPNS($request, $expected, $ns) :mixed{
  VSOAPEX($request,
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
          "<SOAP-ENV:Envelope xmlns:SOAP-ENV=".
          "\"http://schemas.xmlsoap.org/soap/envelope/\"".
          " xmlns:ns1=\"" .$ns. "\"".
          " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"".
          " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"".
          " xmlns:SOAP-ENC=".
          "\"http://schemas.xmlsoap.org/soap/encoding/\"".
          " SOAP-ENV:encodingStyle=".
          "\"http://schemas.xmlsoap.org/soap/encoding/\">".
          "<SOAP-ENV:Body>".
          $expected.
          "</SOAP-ENV:Body></SOAP-ENV:Envelope>\n"
          );
}

function VSOAP($request, $expected) :mixed{
  VSOAPNS($request, $expected, "http://testuri.org");
}

abstract final class DvArrayHackArrExtSoapExtSoapPhp {
  public static $server;
}
<<__EntryPoint>>
function entrypoint_ext_soap(): void {

  //////////////////////////////////////////////////////////////////////

  for ($n = 1; $n <= 2; $n++) {
    switch ($n) {
      case 1:
        DvArrayHackArrExtSoapExtSoapPhp::$server = new SoapServer(null, dict["uri" => "http://testuri.org"]);
        break;
      case 2:
        DvArrayHackArrExtSoapExtSoapPhp::$server = new SoapServer(null, dict["uri" => "http://testuri.org",
                      "ssl_method" => SOAP_SSL_METHOD_TLS]);
        break;
    }

   DvArrayHackArrExtSoapExtSoapPhp::$server->addFunction("hello");

    VSOAP("<ns1:hello xmlns:ns1=\"http://testuri.org\" />",
    "<ns1:helloResponse><return xsi:type=\"xsd:string\">Hello World".
    "</return></ns1:helloResponse>");

    DvArrayHackArrExtSoapExtSoapPhp::$server->addFunction(SOAP_FUNCTIONS_ALL);

    VSOAP("<ns1:strlen xmlns:ns1=\"http://testuri.org\">".
    "<x xsi:type=\"xsd:string\">Hello World</x>".
    "</ns1:strlen>",
    "<ns1:strlenResponse><return xsi:type=\"xsd:int\">11".
    "</return></ns1:strlenResponse>");

    $funcs = vec["sub", "add"];
    DvArrayHackArrExtSoapExtSoapPhp::$server->addFunction($funcs);

    VSOAP("<ns1:add xmlns:ns1=\"http://testuri.org\">".
    "<x xsi:type=\"xsd:int\">22</x>".
    "<y xsi:type=\"xsd:int\">33</y>".
    "</ns1:add>",
    "<ns1:addResponse><return xsi:type=\"xsd:int\">55".
    "</return></ns1:addResponse>");

    DvArrayHackArrExtSoapExtSoapPhp::$server->addFunction("sum");

    VSOAP("<ns1:sum xmlns:ns1=\"http://testuri.org\">".
    "<param0 SOAP-ENC:arrayType=\"xsd:int[2]\"".
    " xmlns:SOAP-ENC=".
    "\"http://schemas.xmlsoap.org/soap/encoding/\"".
    " xsi:type=\"SOAP-ENC:Array\">".
    "  <val xsi:type=\"xsd:int\">3</val>".
    "  <val xsi:type=\"xsd:int\">5</val>".
    "</param0>".
    "</ns1:sum>",
    "<ns1:sumResponse><return xsi:type=\"xsd:int\">8".
    "</return></ns1:sumResponse>");

    DvArrayHackArrExtSoapExtSoapPhp::$server = new SoapServer(__DIR__."/1809.wsdl",
          dict["uri" => "http://testuri.org"]);

    DvArrayHackArrExtSoapExtSoapPhp::$server->addFunction("fault");

    // TODO(#2512715): this doesn't work.
    if (false) {
      VSOAPEX("<ns1:fault xmlns:ns1=\"http://testuri.org\"/>",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
        "<SOAP-ENV:Envelope xmlns:SOAP-ENV=".
        "\"http://schemas.xmlsoap.org/soap/envelope/\"".
        "><SOAP-ENV:Body><SOAP-ENV:fault><faultcode>MyFault</faultcode>".
        "<faultstring>My fault string</faultstring></SOAP-ENV:fault>".
        "</SOAP-ENV:Body></SOAP-ENV:Envelope>\n");
    }
  }
}
