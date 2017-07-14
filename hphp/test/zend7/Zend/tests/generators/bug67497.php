<?php

function gen() {
    $a = 1;
    yield $a;
}

try {
	eval('abc');
} catch (ParseError $ex) {
}

$values = gen();
$values->next();

?>
===DONE===
