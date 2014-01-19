<?php
function foo($f, $t) {
    for ($i = $f; $i <= $t; $i++) {
        try {
            throw new Exception;
        } finally {
            yield $i;
        }
    }
}
foreach (foo(1, 5) as $x) {
    echo $x, "\n";
}