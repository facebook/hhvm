<?php

$sp = new SplPriorityQueue();

$sp->insert("1",1);

$sp->extract(1); // Should throw a warning as extract expects NO arguments

?>