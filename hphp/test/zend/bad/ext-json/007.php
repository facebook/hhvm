<?php
var_dump(json_decode("[1]"));
var_dump(json_last_error());
var_dump(json_decode("[[1]]", false, 2));
var_dump(json_last_error());
var_dump(json_decode("[1}"));
var_dump(json_last_error());
var_dump(json_decode('["' . chr(0) . 'abcd"]'));
var_dump(json_last_error());
var_dump(json_decode("[1"));
var_dump(json_last_error());


echo "Done\n";
?>