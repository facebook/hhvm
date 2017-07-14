<?php

define("FILE_STREAM", fopen("php://temp", "r"));

define("FILE_STREAMS", array(fopen("php://temp", "r")));

var_dump(FILE_STREAM);
var_dump(FILE_STREAMS);
?>
