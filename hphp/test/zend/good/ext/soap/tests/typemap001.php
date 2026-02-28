<?hh

function book_from_xml($xml) :mixed{
	$sxe = simplexml_load_string($xml);
	$obj = new book;
	$obj->a = (string)$sxe->a;
	$obj->b = (string)$sxe->b;
	return $obj;
}

class test{
	function dotest($book):mixed{
		$classname=get_class($book);
		return "Object: ".$classname. "(".$book->a.",".$book->b.")";
	}
}

class book{
	public $a="a";
	public $b="c";

}
<<__EntryPoint>>
function entrypoint_typemap001(): void {
  \HH\global_set('HTTP_RAW_POST_DATA', "
  <env:Envelope xmlns:env=\"http://schemas.xmlsoap.org/soap/envelope/\"
  	xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
  	xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
  	xmlns:enc=\"http://schemas.xmlsoap.org/soap/encoding/\"
  	xmlns:ns1=\"http://schemas.nothing.com\"
  >
    <env:Body>
   <ns1:dotest>
    <book xsi:type=\"ns1:book\">
      <a xsi:type=\"xsd:string\">foo</a>
      <b xsi:type=\"xsd:string\">bar</b>
  </book>
  </ns1:dotest>
   </env:Body>
  <env:Header/>
  </env:Envelope>");
  $options=dict[
  		'actor'   =>'http://schemas.nothing.com',
  		'typemap' => vec[dict["type_ns"   => "http://schemas.nothing.com",
  		                         "type_name" => "book",
  		                         "from_xml"  => book_from_xml<>]]
  		];

  $server = new SoapServer(dirname(__FILE__)."/classmap.wsdl",$options);
  $server->setClass("test");
  $server->handle(\HH\global_get('HTTP_RAW_POST_DATA'));
  echo "ok\n";
}
