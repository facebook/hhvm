<?hh
class MyDOM extends DOMDocument {}


<<__EntryPoint>>
function main_xpath_extended_doc_class() :mixed{
$dom = new MyDOM();
$xpath = new DOMXPath($dom);
var_dump($dom === $xpath->document);

$dom2 = new DOMDocument();
$xpath2 = new DOMXPath($dom2);
var_dump($dom2 === $xpath2->document);

var_dump(get_class($dom), get_class($dom2));
}
