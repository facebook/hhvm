<?php
function & foo() {
    $var = 'ok';
    return $var;
}

function & bar() {
    return foo();
}

$a =& bar();
echo "$a\n";
?>