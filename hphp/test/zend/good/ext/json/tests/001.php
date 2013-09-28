<?php

var_dump(json_decode());
var_dump(json_decode(""));
var_dump(json_decode("", 1));
var_dump(json_decode("", 0));
var_dump(json_decode(".", 1));
var_dump(json_decode(".", 0));
var_dump(json_decode("<?>"));
var_dump(json_decode(";"));
var_dump(json_decode("руссиш"));
var_dump(json_decode("blah"));
var_dump(json_decode(NULL));
var_dump(json_decode('{ "test": { "foo": "bar" } }'));
var_dump(json_decode('{ "test": { "foo": "" } }'));
var_dump(json_decode('{ "": { "foo": "" } }'));
var_dump(json_decode('{ "": { "": "" } }'));
var_dump(json_decode('{ "": { "": "" }'));
var_dump(json_decode('{ "": "": "" } }'));

?>
===DONE===