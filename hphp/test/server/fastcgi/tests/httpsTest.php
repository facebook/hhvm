<?hh

function httpsTestController($serverPort) :mixed{
  $args = dict['HTTPS' => ''];
  var_dump(request('localhost', $serverPort, "test_https.php",
                  dict[], dict[], $args));
}
<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  runTest(httpsTestController<>);
}
