<?hh
<<__EntryPoint>> function main(): void {
// create dom document
$dom = new DOMDocument;
$xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!DOCTYPE s1 PUBLIC "http://www.ibm.com/example.dtd" "example.dtd">
<s1>foo</s1>';
$dom->loadXML($xml);
if(!$dom) {
  echo "Error while parsing the document\n";
  exit;
}
echo "DOMDocument created\n";

$test = $dom->documentURI;
echo "Read initial documentURI:\n";
echo $test."\n";

$dom->documentURI = 'http://dom.example.org/example.xml';
$test = $dom->documentURI;
echo "Set documentURI to a URL, reading again:\n";
var_dump( $test );

echo "Done\n";
}
