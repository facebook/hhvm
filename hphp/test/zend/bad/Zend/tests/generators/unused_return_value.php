<?php

function gen($foo) { yield; }

gen('foo'); // return value not used

?>
===DONE===