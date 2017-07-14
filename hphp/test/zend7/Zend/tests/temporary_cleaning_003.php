<?php

function ops() {
    throw new Exception();
}

try{
    $x = 1;
    $r = [$x] + ops();
} catch (Exception $e) {
}

?>
==DONE==
