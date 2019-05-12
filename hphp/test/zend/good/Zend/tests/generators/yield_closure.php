<?php

function gen() {
    yield function() {};
}
<<__EntryPoint>> function main() {
$gen = gen();
$gen->next();

echo "Done!";
}
