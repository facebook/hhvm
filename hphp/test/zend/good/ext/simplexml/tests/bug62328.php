<?hh
class UberSimpleXML extends SimpleXMLElement {
    public function __toString() :mixed{
        return 'stringification';
    }
}
<<__EntryPoint>> function main(): void {
$xml = new UberSimpleXML('<xml/>');

var_dump((string) $xml);
var_dump($xml->__toString());
}
