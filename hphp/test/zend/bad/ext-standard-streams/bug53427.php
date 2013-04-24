<?php
$read[1] = fopen(__FILE__, "r");
$read["myindex"] = reset($read);
$write = NULL;
$except = NULL;

var_dump($read);

stream_select($read, $write, $except, 0);

var_dump($read);