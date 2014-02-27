<?php

function main() {
  $doc = new DOMDocument();
  $root = $doc->createElement('root');
  var_dump($doc);
  var_dump($root->ownerDocument);
  var_dump($root->ownerDocument === $doc);
}

main();
