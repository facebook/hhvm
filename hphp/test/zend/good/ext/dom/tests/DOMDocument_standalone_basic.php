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
echo "Standalone DOMDocument created\n";

$test = $dom->standalone;
echo "Read initial standalone:\n";
var_dump( $test );

$dom->standalone = FALSE;
$test = $dom->standalone;
echo "Set standalone to FALSE, reading again:\n";
var_dump( $test );

$test = $dom->saveXML();
echo "Document is no longer standalone\n";
var_dump( $test );

echo "Done";
}
