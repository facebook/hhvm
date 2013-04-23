<?php
class UberSimpleXML extends SimpleXMLElement {
    public function __toString() {
        return 'stringification';
    }
}

$xml = new UberSimpleXML('<xml/>');

var_dump((string) $xml);
var_dump($xml->__toString());