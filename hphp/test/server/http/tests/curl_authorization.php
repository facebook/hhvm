<?hh

<<__EntryPoint>>
function main() :mixed{
  require_once('test_base.inc');
  init();
  runTest(
    function($server_port) {
      $url = 'http://localhost:'.$server_port.'/test_authorization_header.php';

      // Send foobar:baz
      $ch = curl_init();
      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_USERPWD, 'foobar:baz');
      curl_exec($ch);
      curl_close($ch);

      // Send the empty string
      $ch = curl_init();
      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_USERPWD, '');
      curl_exec($ch);
      curl_close($ch);

      // Set null after setting a value
      $ch = curl_init();
      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_USERPWD, 'helloworld');
      curl_setopt($ch, CURLOPT_USERPWD, null);
      curl_exec($ch);
      curl_close($ch);
    }
  );
}
