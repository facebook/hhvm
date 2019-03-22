<?php

var_dump(json_decode('{"":"value"}', true));
var_dump(json_decode('{"":"value", "key":"value"}', true));
var_dump(json_decode('{"key":"value", "":"value"}', true));

echo "Done\n";
