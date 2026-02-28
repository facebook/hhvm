<?hh

class ThingWithParent {
  public $parent;
  public $id;
  public $children;
  function __construct($id, $parent) {
    $this->id = $id;
    $this->parent = $parent;
  }
}


class MultiRefTest {
  public function getThingWithParent() :mixed{
    $p = new ThingWithParent(1, null);
    $p2 = new ThingWithParent(2, $p);
    $p3 = new ThingWithParent(3, $p);

    $p->children = vec[$p2, $p3];

    return $p2;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $request = <<<REQUEST
<?xml version="1.0" encoding="UTF-8"?><soapenv:Envelope
xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
xmlns:xsd="http://www.w3.org/2001/XMLSchema"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

<soapenv:Body>
<ns2:getThingWithParent
 soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
 xmlns:ns2="urn:test.soapserver#"/>
</soapenv:Body>

</soapenv:Envelope>
REQUEST;

  $server = new SoapServer(dirname(__FILE__)."/bug37013.wsdl");
  $server->setClass("MultiRefTest");
  $server->handle($request);
}
