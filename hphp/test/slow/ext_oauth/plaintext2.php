<?hh
require 'server.inc';
<<__EntryPoint>> function main(): void {
$x = new OAuth('conskey', 'conssecret', OAUTH_SIG_METHOD_PLAINTEXT);
$x->setRequestEngine(OAUTH_REQENGINE_STREAMS);
$x->setTimestamp(12345);
$x->setNonce('testing');

$output = null;
$port = random_free_port();
$pid = http_server("tcp://127.0.0.1:$port", vec[
    "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 40\r\n\r\noauth_token=1234&oauth_token_secret=4567",
], inout $output);

$x->setAuthType(OAUTH_AUTH_TYPE_URI);
$x->setToken("key", "secret");
var_dump($x->getAccessToken("http://127.0.0.1:$port/test"));

fseek($output, 0, SEEK_SET);
var_dump(stream_get_contents($output));

http_server_kill($pid);
}
