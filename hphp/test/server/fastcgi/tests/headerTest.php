<?hh

function headerTestController($serverPort) :mixed{
  $args = dict['Authorization' => 'foo'];
  var_dump(fastcgi_request('localhost', $serverPort, "test_headers.php",
                  dict[], dict['PROXY' => 'foobar'], $args));
}
<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  runTest(headerTestController<>);
}
