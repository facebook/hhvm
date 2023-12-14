<?hh

function createSocketStream($serverPort) :mixed{
  $errorCode = null;
  $errorMessage = null;
  $stream = stream_socket_client(
    php_uname('n').":".$serverPort,
    inout $errorCode,
    inout $errorMessage,
    3.0
  );
  if (!$stream) {
      exit($errorMessage);
  }
  return $stream;
}

function createReq():mixed{
  $host_name = "hphpd.debugger.".php_uname('n');
  $message = "GET /large_response.php HTTP/1.1\r\n".
             "Host: $host_name\r\nContent-Length:0\r\n\r\n";
  return $message;
}
<<__EntryPoint>> function main(): void {
require_once('test_base.inc');
init();
runTest(function ($serverPort) {
  $stream = createSocketStream($serverPort);
  fwrite($stream, createReq());
  $data = stream_get_contents($stream, 1024 * 1024);
  var_dump(strlen($data));
  fclose($stream);
});

runTest(function ($serverPort) {
  $stream = createSocketStream($serverPort);
  fwrite($stream, createReq());
  $data = fread($stream, 1024 * 1024);
  var_dump(strlen($data));
  fclose($stream);
});
}
