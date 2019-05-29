<?php

function Test ($a) {
    if ($a<3) {
        return(3);
    }
}

<<__EntryPoint>> function main() {
$a = 1;

if ($a < Test($a)) {
    echo "$a\n";
    $a++;
}
}
