<?php

function get() {
    $t = new stdClass;
    $t->prop = $t;
    return $t;
}

$i = 42;
get()->prop =& $i;

?>
===DONE===
