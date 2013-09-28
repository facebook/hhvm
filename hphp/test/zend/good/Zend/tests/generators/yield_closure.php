<?php

function gen() {
    yield function() {};
}

$gen = gen();
$gen->next();

echo "Done!";

?>