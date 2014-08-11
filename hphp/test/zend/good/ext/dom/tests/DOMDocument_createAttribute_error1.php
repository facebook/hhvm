<?php

$doc = new DOMDocument;

$node = $doc->createElement("para");
$newnode = $doc->appendChild($node);

try {
    $failed_test_attribute = $doc->createAttribute("ha haha");
    $node->appendChild($failed_test_attribute);

    echo $doc->saveXML();
}
catch (DOMException $e) {
    echo 'Test failed!', PHP_EOL;
}

?>
