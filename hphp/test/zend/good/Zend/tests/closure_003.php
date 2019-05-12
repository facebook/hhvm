<?php

function run () {
    $x = 4;

    $lambda1 = function () use ($x) {
        echo "$x\n";
    };

    $lambda1();
    $x++;
    $lambda1();
}
<<__EntryPoint>> function main() {
run();

echo "Done\n";
}
