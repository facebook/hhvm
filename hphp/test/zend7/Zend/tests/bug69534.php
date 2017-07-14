<?php
class Node extends SplObjectStorage {
    public $prop;
}

$node1 = new Node;
$node2 = new Node;
$node1->prop = $node2;
$node2->prop = $node1;
unset($node1);
unset($node2);
var_dump(gc_collect_cycles());
?>
