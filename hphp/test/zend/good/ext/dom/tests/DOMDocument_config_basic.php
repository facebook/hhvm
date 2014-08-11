<?php
// create dom document
$dom = new DOMDocument;
echo "DOMDocument created\n";

$test = $dom->config;
echo "Read config:\n";
var_dump( $test );

// note -- will always be null as DOMConfiguration is not implemented in PHP

echo "Done\n";
?>
