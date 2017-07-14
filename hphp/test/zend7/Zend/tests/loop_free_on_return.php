<?php

$a = [42];
foreach ($a as $b) {
    while (1) {
        break 2;
    }
    return;
}
?>
===DONE===
