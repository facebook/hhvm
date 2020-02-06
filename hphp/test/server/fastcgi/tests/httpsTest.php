<?hh

require_once('test_base.inc');

function httpsTestController($serverPort) {
  $args = darray['HTTPS' => ''];
  var_dump(request('localhost', $serverPort, "test_https.php",
                  darray[], darray[], $args));
}
<<__EntryPoint>> function main(): void {
runTest("httpsTestController");
}
