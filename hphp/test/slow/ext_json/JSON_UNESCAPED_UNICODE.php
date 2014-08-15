<?php

$test = html_entity_decode('&#x1D11E;');

var_dump($test);
var_dump(json_encode($test));
var_dump(json_encode($test, JSON_UNESCAPED_UNICODE));
