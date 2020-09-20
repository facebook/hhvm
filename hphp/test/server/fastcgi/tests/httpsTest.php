<?hh

function httpsTestController($serverPort) {
  $args = darray['HTTPS' => ''];
  var_dump(request('localhost', $serverPort, "test_https.php",
                  darray[], darray[], $args));
}
<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  runTest("httpsTestController");
}
