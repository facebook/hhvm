<?php
$a = new SplDoublyLinkedList();
$a->push(1);
$a->push(2);
$a->push(3);

$a->rewind();
while ($a->valid()) {
    var_dump($a->current(), $a->next());
}
?>
===DONE===
<?php exit(0); ?>