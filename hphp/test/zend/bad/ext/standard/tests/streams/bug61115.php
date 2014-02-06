<?php

$arrayLarge = array_fill(0, 113663, '*');

$resourceFileTemp = fopen('php://temp', 'r+');
stream_context_set_params($resourceFileTemp, array());
preg_replace('', function() {}, $resourceFileTemp);
?>