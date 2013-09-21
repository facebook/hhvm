<?php

function gen() { yield; }

$gen = gen();
foreach ($gen as &$value) { }

?>