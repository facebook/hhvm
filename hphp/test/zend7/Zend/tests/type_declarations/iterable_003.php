<?php

function foo(): iterable {
	return [];
}
function bar(): iterable {
	return (function () { yield; })();
}

function baz(): iterable {
    return 1;
}

var_dump(foo());
var_dump(bar());

try {
    baz();
} catch (Throwable $e) {
    echo $e->getMessage();
}

?>
