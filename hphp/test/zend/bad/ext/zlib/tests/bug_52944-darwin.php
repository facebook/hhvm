<?php
require dirname(__FILE__) . "/bug_52944_corrupted_data.inc";

$fp = fopen('data://text/plain;base64,' . $data, 'r');
stream_filter_append($fp, 'zlib.inflate', STREAM_FILTER_READ);
var_dump(fread($fp,1));
var_dump(fread($fp,1));
fclose($fp);
echo "Done.\n";