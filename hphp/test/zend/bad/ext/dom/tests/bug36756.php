<?php

/* Node is preserved from removeChild */
$dom = new DOMDocument();
$dom->loadXML('<root><child/></root>');
$xpath = new DOMXpath($dom);
$node = $xpath->query('/root')->item(0);
echo $node->nodeName . "\n";
$dom->removeChild($GLOBALS['dom']->firstChild);
echo "nodeType: " . $node->nodeType . "\n";

/* Node gets destroyed during removeChild */
$dom->loadXML('<root><child/></root>');
$xpath = new DOMXpath($dom);
$node = $xpath->query('//child')->item(0);
echo $node->nodeName . "\n";
$GLOBALS['dom']->removeChild($GLOBALS['dom']->firstChild);

echo "nodeType: " . $node->nodeType . "\n";

?>
