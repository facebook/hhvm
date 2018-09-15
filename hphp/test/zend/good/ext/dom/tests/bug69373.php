<?php
$doc = new DOMDocument();
for( $i=0; $i<10; $i++ ) {
	$doc->loadXML("<parent><child /><child /></parent>");
	$xpath = new DOMXpath($doc);
	$all = $xpath->query('//*');
	if ($i % 2) {
		$doc->firstChild->nodeValue = 'text';
	} else {
		$doc->firstChild->textContent = 'text';
	}
	if ($i % 2) {
		var_dump($doc->firstChild->textContent);
	} else {
		var_dump($doc->firstChild->nodeValue);
	}
}
echo 'DONE', PHP_EOL;
?>

