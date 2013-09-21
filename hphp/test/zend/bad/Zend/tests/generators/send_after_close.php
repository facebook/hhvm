<?php

function gen() { var_dump(yield); }

$gen = gen();
$gen->send('foo');
$gen->send('bar');

?>