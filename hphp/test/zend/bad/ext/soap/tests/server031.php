<?php
class ItemArray implements Iterator {
	private $a = array();

	public function __construct(array $a) {
		$this->a = $a;
	}

    public function rewind()    { return reset($this->a); }
    public function current()   { return current($this->a); }
    public function key()       { return key($this->a); }
    public function next()      { return next($this->a); }
    public function valid()     { return (current($this->a) !== false); }
}

class Item {
    public $text;

	public function __construct($n) {
		$this->text = 'text'.$n;
	}
}

class handlerClass {
    public function getItems()
    {
        return new ItemArray(array(
        		new Item(0),
        		new Item(1),
        		new Item(2),
        		new Item(3),
        		new Item(4),
        		new Item(5),
        		new Item(6),
        		new Item(7),
        		new Item(8),
        		new Item(9)
        	));
    }
}

$server = new SoapServer(dirname(__FILE__)."/server030.wsdl");
$server->setClass('handlerClass');

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/">
  <SOAP-ENV:Body>
    <getItems/>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;

$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";
?>