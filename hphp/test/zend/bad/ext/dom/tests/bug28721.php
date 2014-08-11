<?php
function print_node(DomNode $node) {
  echo "name (value): " . $node->nodeName . " (" . $node->nodeValue . ")\n";
}

function print_node_r(DomNode $node) {
  static $indent = "";
  echo "\n" . $indent;
  print_node($node);

  echo $indent . "parent: ";
  if ( $node->parentNode )
    print_node($node->parentNode);
  else
    echo "NULL\n";

  echo $indent . "previousSibling: ";
  if ( $node->previousSibling )
    print_node($node->previousSibling);
  else
    echo "NULL\n";
  
  echo $indent . "nextSibling: ";
  if ( $node->nextSibling )
    print_node($node->nextSibling);
  else
    echo "NULL\n";

  if ( !$node->hasChildNodes() )
    return;
  
  foreach ($node->childNodes as $child) {

    $old_indent  = $indent;
    $indent .= "  ";
    print_node_r($child);
    $indent = $old_indent;
  }
}

function err_handler($errno, $errstr, $errfile, $errline) {
  echo "Error ($errno) on line $errline: $errstr\n";
}

// Record 'DocumentFragment is empty' warnings
set_error_handler("err_handler", E_WARNING);

$xml = new DomDocument();

$p = $xml->createElement("p");

$p->appendChild($t1 = $xml->createTextNode(" t1 "));
$p->appendChild($b = $xml->createElement("b"));
$b->appendChild($xml->createTextNode("X"));
$p->appendChild($t2 = $xml->createTextNode(" t2 "));
$p->appendChild($xml->createTextNode(" xxx "));

print_node_r($p);

echo "\nAppend t1 to p:\n";
$ret = $p->appendChild($t1);

print_node_r($p);
echo "\n";

echo "t1 == ret: ";
var_dump( $t1 === $ret );


$d = $xml->createElement("div");
$d->appendChild($t3 = $xml->createTextNode(" t3 "));
$d->appendChild($b = $xml->createElement("b"));
$b->appendChild($xml->createElement("X"));
$d->appendChild($t4 = $xml->createTextNode(" t4 "));
$d->appendChild($xml->createTextNode(" xxx "));

echo "\ndiv:\n";
print_node_r($d);

echo "\nInsert t4 before t3:\n";

$ret = $d->insertBefore($t4, $t3);

print_node_r($d);
echo "\n";

$frag = $xml->createDocumentFragment();

$t5 = $frag->appendChild($xml->createTextNode(" t5 "));
$frag->appendChild($i = $xml->createElement("i"));
$i->appendChild($xml->createTextNode(" frob "));
$frag->appendChild($xml->createTextNOde(" t6 "));

echo "\np:\n";
print_node_r($p);
echo "\nFragment:\n";
print_node_r($frag);

echo "\nAppending fragment to p:\n";
$p->appendChild($frag);

print_node_r($p);
echo "\nFragment:\n";
print_node_r($frag);

echo "\ndiv:\n";
print_node_r($d);
echo "\nInserting fragment before t4\n";
$d->insertBefore($frag, $t4);
print_node_r($d);

echo "\np:\n";
print_node_r($p);

?>
