<?hh

function add($a, $b) :mixed{
 return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($b);
 }

<<__EntryPoint>>
function main_headers_sent_no_warn() :mixed{
var_dump(headers_sent()); // false
var_dump(headers_sent()); //true
$server = new SoapServer(NULL, dict['uri' => 'http://test-uri']);
$str = '<?xml version="1.0" '
.       'encoding="ISO-8859-1"?>'
.       '<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='
.       '"http://schemas.xmlsoap.org/soap/encoding/"'
.       ' xmlns:SOAP-ENV='
.       '"http://schemas.xmlsoap.org/soap/envelope/"'
.       ' xmlns:xsd="http://www.w3.org/2001/XMLSchema"'
.       ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
.       ' xmlns:si="http://soapinterop.org/xsd"><SOAP-ENV:Body>'
.       '<ns1:add xmlns:ns1="http://testuri.org">'
.       '<x xsi:type="xsd:hexBinary">16</x>'
.       '<y xsi:type="xsd:hexBinary">21</y>'
.       '</ns1:add>  </SOAP-ENV:Body></SOAP-ENV:Envelope>';
$server->addFunction('add');
$server->handle($str);
var_dump(headers_sent()); // true
var_dump(headers_list()); // empty
}
