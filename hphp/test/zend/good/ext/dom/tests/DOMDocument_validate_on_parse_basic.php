<?php

require_once('dom_test.inc');

$dom = new DOMDocument;
$dom->loadXML($xmlstr);

if( !$dom )
{
    echo "Error while parsing the document\n";
    exit;
}

echo "Checking documented default value: ";
var_dump($dom->validateOnParse);

$dom->validateOnParse = TRUE;
echo "Setting validateOnParse to TRUE: ";
var_dump($dom->validateOnParse);

$dom->validateOnParse = FALSE;
echo "Setting validateOnParse to FALSE: ";
var_dump($dom->validateOnParse);

?>
