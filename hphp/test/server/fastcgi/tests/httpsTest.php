<?hh

require_once('test_base.inc');

function httpsTestController($serverPort) {
  $args = array('HTTPS' => '');
  var_dump(request('localhost', $serverPort, "test_https.php",
                  [], [], $args));
}
<<__EntryPoint>> function main(): void {
runTest("httpsTestController");
}
