<?php

foreach ([0] as $x) {
    goto a;
a:
    echo "loop\n";
}

echo "done\n";
?>
