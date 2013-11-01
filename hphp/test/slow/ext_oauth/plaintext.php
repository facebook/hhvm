<?php
require 'server.inc';

$x = new OAuth('conskey', 'conssecret', OAUTH_SIG_METHOD_PLAINTEXT);
$x->setRequestEngine(OAUTH_REQENGINE_STREAMS);
$x->setTimestamp(12345);
$x->setNonce('testing');

$port = random_free_port();
$pid = http_server("tcp://127.0.0.1:$port", array(
	"HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 40\r\n\r\noauth_token=1234&oauth_token_secret=4567",
), $output);

$x->setAuthType(OAUTH_AUTH_TYPE_URI);
var_dump($x->getRequestToken("http://127.0.0.1:$port/test"));

fseek($output, 0, SEEK_SET);
var_dump(stream_get_contents($output));

http_server_kill($pid);
?>
