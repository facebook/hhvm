<?php

//offset to large
$dom = new DomDocument();
$comment = $dom->createComment('test-comment');
try {
  $comment->insertData(999,'-inserted');
} catch (DOMException $e ) {
  if ($e->getMessage() == 'Index Size Error'){
    echo "Throws DOMException for offset too large\n";
  }
}

?>
