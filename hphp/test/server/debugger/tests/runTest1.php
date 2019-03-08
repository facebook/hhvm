<?hh

require_once('test_base.inc');

function test1Controller($hphpdOutput, $hphpdProcessId, $serverPort) {
  // Request a page so that the client can debug it.
  waitForClientToOutput($hphpdOutput, "Waiting for server response");
  $url = "http://".php_uname('n').':'.$serverPort.'/test1.php';
  echo "Requesting test1.php\n";
  request(php_uname('n'), $serverPort, 'test1.php', 10); // ignore response

  // Let client run until script quits
  waitForClientToOutput($hphpdOutput, "quit");
}

runTest('test1', "test1Controller");
