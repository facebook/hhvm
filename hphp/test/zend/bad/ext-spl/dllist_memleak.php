<?php
$dll = new SplDoublyLinkedList();
$dll->push(1);
$dll->push(2);
$dll->push(3);
$dll->push(4);


$dll->rewind();
echo $dll->current()."\n";
$dll->next();
$dll->next();
echo $dll->current()."\n";

?>
===DONE===
<?php exit(0); ?>