<?php

function foo() {
    try { return str_repeat("a", 2); }
    finally { return str_repeat("b", 2); }
}

var_dump(foo());
?>
