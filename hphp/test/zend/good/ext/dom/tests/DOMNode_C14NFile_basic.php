<?hh <<__EntryPoint>> function main(): void {
$xml = <<< XML
<?xml version="1.0" ?>
<books>
 <book>
  <title>The Grapes of Wrath</title>
  <author>John Steinbeck</author>
 </book>
 <book>
  <title>The Pearl</title>
  <author>John Steinbeck</author>
 </book>
</books>
XML;

$output = __SystemLib\hphp_test_tmppath('DOMNode_C14NFile_basic.tmp');
$doc = new DOMDocument();
$doc->loadXML($xml);
$node = $doc->getElementsByTagName('title')->item(0);
var_dump($node->C14NFile($output));
$content = file_get_contents($output);
var_dump($content);

unlink($output);
}
