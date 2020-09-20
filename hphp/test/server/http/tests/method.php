<?hh

<<__EntryPoint>>
function main() {
  require_once('test_base.inc');
  init();
  runTest(
    function($server_port) {
      echo http_request(
        'localhost',
        $server_port,
        'method.php',
        1200,
        '-X MOVE',
      );
    }
  );
}
