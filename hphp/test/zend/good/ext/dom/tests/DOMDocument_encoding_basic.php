<?php

require_once('dom_test.inc');

$dom = new DOMDocument;
$dom->loadXML($xmlstr);

if( !$dom )
{
    echo "Error while parsing the document\n";
    exit;
}

echo "Empty Encoding Read: {$dom->encoding}\n";

$ret = $dom->encoding = 'NYPHP DOMinatrix';
echo "Adding invalid encoding: $ret\n";

$ret = $dom->encoding = 'ISO-8859-1';
echo "Adding ISO-8859-1 encoding: $ret\n";
echo "ISO-8859-1 Encoding Read: {$dom->encoding}\n";

$ret = $dom->encoding = 'UTF-8';
echo "Adding UTF-8 encoding: $ret\n";
echo "UTF-8 Encoding Read: {$dom->encoding}\n";

$ret = $dom->encoding = 'UTF-16';
echo "Adding UTF-16 encoding: $ret\n";
echo "UTF-16 Encoding Read: {$dom->encoding}\n";


?>
