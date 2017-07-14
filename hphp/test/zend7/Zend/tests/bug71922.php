<?php

try {
    assert(0 && new class {
    } && new class(42) extends stdclass {
    });
} catch (AssertionError $e) {
    echo "Assertion failure: ", $e->getMessage(), "\n";
}

?>
