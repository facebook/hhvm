<?hh
class ItemArray implements Iterator {


    public function __construct(private array $arr) {

    }

    public function rewind()  { $arr = $this->arr; reset(inout $arr); $this->arr = $arr;  }
    public function current() { $arr = $this->arr; $x = current($arr); $this->arr = $arr; return $x; }
    public function key()     { $arr = $this->arr; $key = key($arr); $this->arr = $arr; return $key; }
    public function next()    { $arr = $this->arr; $n = next(inout $arr); $this->arr = $arr; return $n; }
    public function valid()   { $arr = $this->arr; $current = current($arr); $this->arr = $arr; return $current !== false; }
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
        return new ItemArray(varray[
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
            ]);
    }
}
<<__EntryPoint>> function main(): void {
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
}
