<?php

$sp = new SplPriorityQueue();

$sp->setExtractFlags(1,1); // Should throw a warning as setExtractFlags expects only 1 argument

?>