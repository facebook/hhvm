<?php

var_dump(stream_wrapper_unregister('file'));
var_dump(fopen("file://".__FILE__, "r"));
var_dump(stream_wrapper_restore('file'));
var_dump(fopen("file://".__FILE__, "r"));

echo "Done\n";
?>