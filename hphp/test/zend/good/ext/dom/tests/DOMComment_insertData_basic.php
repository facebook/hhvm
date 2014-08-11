<?php

//correct offset
$dom = new DomDocument();
$comment = $dom->createComment('test-comment');
$comment->insertData(4,'-inserted');
$dom->appendChild($comment);
echo $dom->saveXML();

?>
