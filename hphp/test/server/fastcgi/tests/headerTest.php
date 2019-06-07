<?hh

require_once('test_base.inc');

function headerTestController($serverPort) {
  $args = array('Authorization' => 'foo');
  var_dump(request('localhost', $serverPort, "test_headers.php",
                  [], ['PROXY' => 'foobar'], $args));
}

runTest("headerTestController");
