<?php
$dom = new domdocument;

$data = file_get_contents(dirname(__FILE__)."/xinclude.xml");
$reldir = str_replace(getcwd(),".",dirname(__FILE__));
if (DIRECTORY_SEPARATOR == '\\') {
	$reldir = str_replace('\\',"/", $reldir);
}
$data = str_replace('compress.zlib://ext/dom/tests/','compress.zlib://'.$reldir."/", $data);


$dom->loadXML($data);
$dom->xinclude();
print $dom->saveXML()."\n";
foreach ($dom->documentElement->childNodes as $node) {
	print $node->nodeName."\n";
}
?>
