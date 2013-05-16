<?php

$a = simplexml_load_string("<a><b><c/></b></a>");
echo $a->getName()."\n";
echo $a->b->getName()."\n";
echo $a->b->c->getName()."\n";
?>
===DONE===