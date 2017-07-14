<?php

function generate() {
    try {
        yield 1;
        yield 2;
    } finally {
        echo "finally\n";
    }
}

foreach (generate() as $i) {
    echo $i, "\n";
    throw new Exception();
}

?>
