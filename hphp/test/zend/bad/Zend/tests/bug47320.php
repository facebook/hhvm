<?php
if (!@substr('no 2nd parameter')) {
    echo '$php_errormsg in global: ' . $php_errormsg . "\n";
}

function foo() {
    if (!@strpos('no 2nd parameter')) {
        echo '$php_errormsg in function: ' . $php_errormsg . "\n";

        echo '$GLOBALS[php_errormsg] in function: ' .
                $GLOBALS['php_errormsg'] . "\n";
    }
}

foo();
?>