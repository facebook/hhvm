<?php

//Negative offset
$dom = new DomDocument();
$comment = $dom->createComment('test-comment');
try {
  $comment->replaceData(-1,4,'-inserted');
} catch (DOMException $e ) {
  if ($e->getMessage() == 'Index Size Error'){
    echo "Throws DOMException for -ve offest\n";
  }
}

?>
