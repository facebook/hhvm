<?php
echo "* Testing JSON output\n\n";
var_dump(json_encode(12.3, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(12, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(12.0, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(0.0, JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode(array(12, 12.0, 12.3), JSON_PRESERVE_ZERO_FRACTION));
var_dump(json_encode((object)array('float' => 12.0, 'integer' => 12), JSON_PRESERVE_ZERO_FRACTION));

echo "\n* Testing encode/decode symmetry\n\n";

var_dump(json_decode(json_encode(12.3, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(12, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(12.0, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(0.0, JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode(array(12, 12.0, 12.3), JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode((object)array('float' => 12.0, 'integer' => 12), JSON_PRESERVE_ZERO_FRACTION)));
var_dump(json_decode(json_encode((object)array('float' => 12.0, 'integer' => 12), JSON_PRESERVE_ZERO_FRACTION), true));
