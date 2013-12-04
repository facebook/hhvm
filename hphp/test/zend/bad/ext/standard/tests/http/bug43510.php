<?php
ini_set('allow_url_fopen', 1);

require 'server.inc';

$responses = array(
	"data://text/plain,HTTP/1.0 200 OK\r\n\r\n",
	"data://text/plain,HTTP/1.0 200 OK\r\n\r\n",
);

$pid = http_server("tcp://127.0.0.1:12342", $responses, $output);

foreach(array('r', 'rb') as $mode) {
	$fd = fopen('http://127.0.0.1:12342/', $mode, false);
	$meta = stream_get_meta_data($fd);
	var_dump($meta['mode']);
	fclose($fd);
}

http_server_kill($pid);

?>