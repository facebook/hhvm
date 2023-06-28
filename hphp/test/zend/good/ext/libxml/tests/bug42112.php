<?hh

function remove_node($doc) :mixed{
    $node = $doc->getElementById( 'id1' );
    print 'Deleting Node: '.$node->nodeName."\n";
    $node->parentNode->removeChild( $node );
}
<<__EntryPoint>>
function entrypoint_bug42112(): void {
  $xml = <<<EOXML
<root><child xml:id="id1">baz</child></root>
EOXML;

  $doc = new DOMDocument();
  $doc->loadXML($xml);

  remove_node($doc);

  $node = $doc->getElementById( 'id1' );
  if ($node) {
  	print 'Found Node: '.$node->nodeName."\n";
  }
  $root = $doc->documentElement;
  print 'Root Node: '.$root->nodeName."\n";
}
