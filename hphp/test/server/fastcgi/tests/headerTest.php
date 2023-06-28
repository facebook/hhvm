<?hh

function headerTestController($serverPort) :mixed{
  $args = darray['Authorization' => 'foo'];
  var_dump(fastcgi_request('localhost', $serverPort, "test_headers.php",
                  darray[], darray['PROXY' => 'foobar'], $args));
}
<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  runTest(headerTestController<>);
}
