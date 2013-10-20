<?php

function make_relative_to_repo($path) {
    $pos = strpos($path, "test/quick");
    return substr($path, $pos);
}

echo make_relative_to_repo(__FILE__)."\n";
echo make_relative_to_repo(__DIR__)."\n";
