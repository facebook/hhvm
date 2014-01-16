<?php
class ItemArray extends ArrayObject {

}

class Item {
    public $text;
}

class handlerClass {
    public function getItems()
    {
        $items = new ItemArray(array());

        for ($i = 0; $i < 10; $i++) {
            $item = new Item();
            $item->text = 'text'.$i;

            $items[] = $item;
        }

        return $items;
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