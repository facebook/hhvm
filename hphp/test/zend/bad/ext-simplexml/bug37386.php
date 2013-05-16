<?php

$sx1 = new SimpleXMLElement((binary)"<root />");

$sx1->node[0] = 'node1';
$sx1->node[1] = 'node2';

print $sx1->asXML()."\n";
$node = $sx1->node[0];
$node[0] = 'New Value';

print $sx1->asXML();

?>