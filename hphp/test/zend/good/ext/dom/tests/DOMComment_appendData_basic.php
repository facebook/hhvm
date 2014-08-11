<?php

$dom = new DomDocument();
$comment = $dom->createComment('test-comment');
$comment->appendData('-more-data');
$dom->appendChild($comment);
$dom->saveXML();
echo $dom->saveXML();

?>
