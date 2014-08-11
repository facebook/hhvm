<?php

require_once('dom_test.inc');

chdir(__DIR__);
$XMLStringGood = file_get_contents(dirname(__FILE__).'/note.xml');

$dom = new DOMDocument;
$dom->resolveExternals = TRUE;

$dom->validateOnParse = FALSE;
echo "validateOnParse set to FALSE: \n";
$dom->loadXML($XMLStringGood);
echo "No Error Report Above\n";

$BogusElement = $dom->createElement('NYPHP','DOMinatrix');
$Body = $dom->getElementsByTagName('from')->item(0);
$Body->appendChild($BogusElement);
$XMLStringBad = $dom->saveXML();

echo "validateOnParse set to TRUE: \n";
$dom->validateOnParse = TRUE;
$dom->loadXML($XMLStringBad);
echo "Error Report Above\n";

?>
