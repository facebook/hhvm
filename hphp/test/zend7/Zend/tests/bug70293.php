<?php

function f() {
    assert(@$a ?: 1);
    echo "OK";
};
f();

?>
