<?php
function test_procedural() {
  $doc = new DOMDocument();

  $root = $doc->createElement('html');
  $doc->appendChild($root);

  $head = $doc->createElement('head');
  dom_node_insert_before($root, $head);

  $body = $doc->createElement('body');
  // Check that an explicit null gets coerced to NullObject, not stdClass
  dom_node_insert_before($root, $body, null);

  var_dump($doc->saveHTML());
}

function test_objects() {
  $doc = new DOMDocument();

  $root = $doc->createElement('html');
  $doc->appendChild($root);

  $head = $doc->createElement('head');
  $root->insertBefore($head);

  $body = $doc->createElement('body');
  $root->insertBefore($body, null);

  var_dump($doc->saveHTML());
}

test_procedural();
test_objects();
