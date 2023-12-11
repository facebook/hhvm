<?hh
require 'server.inc';
<<__EntryPoint>> function main(): void {
$x = new OAuth('1234','1234');
$x->setRequestEngine(OAUTH_REQENGINE_CURL);

$output = null;
$port = random_free_port();
$pid = http_server("tcp://127.0.0.1:$port", vec[
  "HTTP/1.0 302 Found\r\nLocation: http://127.0.0.1:$port/" . str_repeat('a', 512) . "bbb\r\n\r\n",
  "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 40\r\n\r\noauth_token=1234&oauth_token_secret=4567",
], inout $output);

try {
  $x->setAuthType(OAUTH_AUTH_TYPE_AUTHORIZATION);
  var_dump($x->getRequestToken("http://127.0.0.1:$port/test", null, 'GET'));
} catch (Exception $e) {
  var_dump($x->debugInfo);
}
fseek($output, 0, SEEK_SET);

// Split up the requests, make sure we sort the headers
// since we need to compare output but we don't care about
// the exact ordering.
$requests = explode("\r\n\r\n", trim(stream_get_contents($output)));
foreach ($requests as $cur_request) {
  $request_headers = explode("\r\n", $cur_request);
  var_dump(array_shift($request_headers));
  sort($request_headers);
  foreach ($request_headers as $cur_header) {
    var_dump($cur_header);
  }
}

http_server_kill($pid);
}
