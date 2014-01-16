<?php
ini_set('allow_url_fopen', 1);

$fp = fopen(__FILE__, "r");
var_dump($fp);
var_dump(stream_supports_lock($fp));
fclose($fp);

$fp = fopen("file://" . __FILE__, "r");
var_dump($fp);
var_dump(stream_supports_lock($fp));
fclose($fp);

$fp = fopen("php://memory", "r");
var_dump($fp);
var_dump(stream_supports_lock($fp));
fclose($fp);

$fp = fopen('data://text/plain,foobar', 'r');
var_dump($fp);
var_dump(stream_supports_lock($fp));
fclose($fp);

$sock = stream_context_create();
var_dump($sock);
var_dump(stream_supports_lock($sock));

echo "Done\n";
?>