<?hh


<<__EntryPoint>>
function main_ns_null_deref() :mixed{
$xml = new SimpleXMLElement("X",1);
var_dump($xml->getDocNamespaces());
}
