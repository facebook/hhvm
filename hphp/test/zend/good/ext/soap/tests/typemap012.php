<?hh
class TestSoapClient extends SoapClient{
  function __dorequest($request, $location, $action, $version, $one_way = 0) :mixed{
          echo $request;
          exit;
    }
}

class book{
    public $a="a";
    public $b="c";

}

function book_to_xml($book) :mixed{
    throw new SoapFault("Client", "Conversion Error");
}
<<__EntryPoint>> function main(): void {
$options=darray[
        'actor' =>'http://schemas.nothing.com',
        'typemap' => varray[darray["type_ns"   => "http://schemas.nothing.com",
                                 "type_name" => "book",
                                 "to_xml"  => book_to_xml<>]]
        ];

$client = new TestSoapClient(dirname(__FILE__)."/classmap.wsdl",$options);
$book = new book();
$book->a = "foo";
$book->b = "bar";
try {
    $ret = $client->__soapcall('dotest', varray[$book]);
} catch (SoapFault $e) {
    $ret = "SoapFault = " . $e->faultcode . " - " . $e->faultstring;
}
var_dump($ret);
echo "ok\n";
}
