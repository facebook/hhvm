<?php

$it = new ArrayIterator(array());

$lit = new LimitIterator($it, 0, 5);

foreach ($lit as $v) {
    echo $v;
}
echo "===DONE===\n";
