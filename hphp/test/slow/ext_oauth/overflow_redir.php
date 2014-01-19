<?php
require 'server.inc';

$x = new OAuth('1234','1234');
$x->setRequestEngine(OAUTH_REQENGINE_CURL);

$port = random_free_port();
$pid = http_server("tcp://127.0.0.1:$port", array(
	"HTTP/1.0 302 Found\r\nLocation: http://127.0.0.1:$port/" . str_repeat('a', 512) . "bbb\r\n\r\n",
	"HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 40\r\n\r\noauth_token=1234&oauth_token_secret=4567",
), $output);

try {
	$x->setAuthType(OAUTH_AUTH_TYPE_AUTHORIZATION);
	var_dump($x->getRequestToken("http://127.0.0.1:$port/test", null, 'GET'));
} catch (Exception $e) {
	var_dump($x->debugInfo);
}
fseek($output, 0, SEEK_SET);
var_dump(stream_get_contents($output));

http_server_kill($pid);

?>
