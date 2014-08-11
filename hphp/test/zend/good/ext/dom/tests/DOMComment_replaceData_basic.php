<?php

$dom = new DomDocument();
$comment = $dom->createComment('test-comment');
$comment->replaceData(4,1,'replaced');
$dom->appendChild($comment);
echo $dom->saveXML();

// Replaces rest of string if count is greater than length of existing string
$dom = new DomDocument();
$comment = $dom->createComment('test-comment');
$comment->replaceData(0,50,'replaced');
$dom->appendChild($comment);
echo $dom->saveXML();

?>
