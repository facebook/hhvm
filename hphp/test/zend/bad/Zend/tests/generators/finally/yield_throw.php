<?php
function foo($f, $t) {
    for ($i = $f; $i <= $t; $i++) {
        try {
            yield $i;
        } finally {
            throw new Exception;
        }
    }
}
foreach (foo(1, 5) as $x) {
    echo $x, "\n";
}