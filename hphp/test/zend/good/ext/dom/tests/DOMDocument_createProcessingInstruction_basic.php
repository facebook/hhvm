<?php

$doc = new DOMDocument;

$node = $doc->createElement("para");
$newnode = $doc->appendChild($node);

$test_proc_inst0 =
    $doc->createProcessingInstruction( "blablabla" );
$node->appendChild($test_proc_inst0);

$test_proc_inst1 =
    $doc->createProcessingInstruction( "blablabla", "datadata" );
$node->appendChild($test_proc_inst1);

echo $doc->saveXML();

?>
