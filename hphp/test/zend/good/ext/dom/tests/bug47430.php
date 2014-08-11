<?php 
$xml = '<?xml
version="1.0"?><html><p><i>Hello</i></p><p><i>World!</i></p></html>';
$dom = new DOMDocument();
$dom->loadXML($xml);

$elements = $dom->getElementsByTagName('i');
foreach ($elements as $i) {
  $i->previousSibling->nodeValue = '';
}

$arr = array();
$arr[0] = 'Value';

print_r($arr);

?>
