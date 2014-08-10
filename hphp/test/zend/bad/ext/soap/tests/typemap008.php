<?php
class TestSoapClient extends SoapClient{
  function __doRequest($request, $location, $action, $version, $one_way = 0) {
  		echo $request;
  		exit;
	}	
}

class book{
	public $a="a";
	public $b="c";
		
}

function book_to_xml($book) {
	return '<book xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><a xsi:type="xsd:string">'.$book->a.'!</a><b xsi:type="xsd:string">'.$book->b.'!</b></book>';
}

$options=Array(
        'uri'      => 'http://schemas.nothing.com',
        'location' => 'test://',
		'actor'    => 'http://schemas.nothing.com',
		'typemap'  => array(array("type_ns"   => "http://schemas.nothing.com",
		                          "type_name" => "book",
		                          "to_xml"  => "book_to_xml"))
		);

$client = new TestSoapClient(NULL, $options);
$book = new book();
$book->a = "foo";
$book->b = "bar";
$ret = $client->dotest(new SoapVar($book, null, "book", "http://schemas.nothing.com"));
var_dump($ret);
echo "ok\n";
?>
