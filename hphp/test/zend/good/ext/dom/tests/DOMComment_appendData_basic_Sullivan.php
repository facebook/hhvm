<?php

$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);

$comment = $document->createElement('comment');
$root->appendChild($comment);

$commentnode = $document->createComment('');
$comment->appendChild($commentnode);
$commentnode->appendData('data');
echo "Comment Length (one append): " . $commentnode->length . "\n";

$commentnode->appendData('><&"');
echo "Comment Length (two appends): " . $commentnode->length . "\n";

echo "Comment Content: " . $commentnode->data . "\n";

echo "\n" . $document->saveXML();

?>
