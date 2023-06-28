<?hh

function invalidTestController($port) :mixed{
  $host = 'localhost';

  $filename = __DIR__.'/request-doesnotexist.dat';
  $file = fopen($filename, 'rb');
  $req_dat = fread($file, filesize($filename));
  fclose($file);

  // Repeat the data three times, to make it invalid. This particular bytestream
  // (and ones like it -- repeat 3 times!) in particular used to tickle a
  // use-after-free in the FastCGI support.
  $req_dat = $req_dat . $req_dat . $req_dat;

  $errno = null;
  $errstr = null;
  $sock = fsockopen($host, $port, inout $errno, inout $errstr);
  fwrite($sock, $req_dat);
  fclose($sock);

  // Should still be able to recover and respond to a request over the port on a
  // new TCP connection.
  echo request($host, $port, 'hello.php');
  echo "\n";
}
<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  runTest(invalidTestController<>);
}
