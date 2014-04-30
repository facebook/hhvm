<?php
// Adapted from ext/standard/tests/file/stream_001.php
// Removed 2/3 spammy warnings
var_dump(stream_wrapper_unregister('file'));
var_dump(fopen("file://".__FILE__, "r"));
var_dump(stream_wrapper_restore('file'));
var_dump(fopen("file://".__FILE__, "r"));

echo "Done\n";
