<?php

var_dump(json_encode(""));
var_dump(json_encode(NULL));
var_dump(json_encode(TRUE));
var_dump(json_encode(array(""=>"")));
var_dump(json_encode(array(array(1))));

var_dump(json_encode(1));
var_dump(json_encode("руссиш"));


echo "Done\n";
