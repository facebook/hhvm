<?hh

require_once('test_base.inc');

function headerTestController($serverPort) {
  $args = darray['Authorization' => 'foo'];
  var_dump(request('localhost', $serverPort, "test_headers.php",
                  darray[], darray['PROXY' => 'foobar'], $args));
}
<<__EntryPoint>> function main(): void {
runTest("headerTestController");
}
