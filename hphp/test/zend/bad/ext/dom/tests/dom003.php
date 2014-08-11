<?php

$dom = new domdocument;
$dom->load(dirname(__FILE__)."/book.xml");
$rootNode = $dom->documentElement;
print "--- Catch exception with try/catch\n";
try {
    $rootNode->appendChild($rootNode);
} catch (domexception $e) {
	ob_start();
    var_dump($e);
	$contents = ob_get_contents();
	ob_end_clean();
	echo preg_replace('/object\(DOMElement\).+\{.*?\}/s', 'DOMElement', $contents);
}
print "--- Don't catch exception with try/catch\n";
$rootNode->appendChild($rootNode);


?>
