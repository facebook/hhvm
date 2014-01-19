<?php

$ar = new ArrayIterator(array(NULL=>NULL));
@var_dump($ar);
var_dump($ar->getArrayCopy());

?>
===DONE===
<?php exit(0); ?>