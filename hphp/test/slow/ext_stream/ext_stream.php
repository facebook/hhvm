<?hh

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

abstract final class Statics {
  public static $base = -1;
  public static $server = null;
}

//////////////////////////////////////////////////////////////////////

// so we run on different range of ports every time
function get_random_port() {
  if (Statics::$base == -1) {
    Statics::$base = 12345 + (int)((int)(microtime(false) * 100) % 30000);
  }
  return ++Statics::$base;
}

// On the continuous integration server, it's not unlikely that we'll
// fail to bind one of these random ports.  Retry a few times and hope
// for the best.
function retry_bind_server($udp = false) {
  for ($i = 0; $i < 20; ++$i) {
    $port = get_random_port();
    $scheme = $udp ? "udp://" : "tcp://";
    $address = $scheme."127.0.0.1:" . $port;

    $errno = null;
    $errstr = null;
    if ($udp) {
      $server = @stream_socket_server($address, inout $errno, inout $errstr,
                                     STREAM_SERVER_BIND);
    } else {
      $server = @stream_socket_server($address, inout $errno, inout $errstr);
    }
    if ($server !== false) {
      // assing $server into a static property to make sure, it stays alive
      // until next retry_bind_server call.
      Statics::$server = $server;
      return array($port, $address, $server);
    }
  }
  throw new Exception("Couldn't bind server");
}

function retry_bind_server6($udp = false) {
  for ($i = 0; $i < 20; ++$i) {
    $port = get_random_port();
    $scheme = $udp ? "udp://" : "tcp://";
    $address = $scheme."[::1]:" . $port;

    $errno = null;
    $errstr = null;
    if ($udp) {
      $server = @stream_socket_server($address, inout $errno, inout $errstr,
                                     STREAM_SERVER_BIND);
    } else {
      $server = @stream_socket_server($address, inout $errno, inout $errstr);
    }
    if ($server !== false) {
      // assing $server into a static property to make sure, it stays alive
      // until next retry_bind_server call.
      Statics::$server = $server;
      return array($port, $address, $server);
    }
  }
  throw new Exception("Couldn't bind server");
}

///////////////////////////////////////////////////////////////////////////////

function test_stream_copy_to_stream() {
  $src = fopen(__DIR__."/../ext_file/test_ext_file.txt", "r");
  $dtmp = tempnam('/tmp', 'vmcopystream');
  $dest = fopen($dtmp, "w");
  stream_copy_to_stream($src, $dest);
  fclose($dest);

  $f = fopen($dtmp, "r");
  VS(fgets($f), "Testing Ext File\n");

  unlink($dtmp);
}

function test_stream_get_contents() {
  $f = fopen(__DIR__."/../ext_file/test_ext_file.txt", "r");
  VS(stream_get_contents($f), "Testing Ext File\n");

  $f = tmpfile();

  fwrite($f, "fwrite1");
  fseek($f, 0); VS(stream_get_contents($f), "fwrite1");

  fwrite($f, "fwrite2");
  fseek($f, 0); VS(stream_get_contents($f), "fwrite1fwrite2");

  fwrite($f, "fwrite3");
  VS(stream_get_contents($f), "");

  fclose($f);
}

function test_stream_get_line() {
  $f = fopen(__DIR__."/../ext_file/test_ext_file.txt", "r");
  VS(stream_get_line($f), "Testing Ext File\n");

  $f = tmpfile();
  fwrite($f, "stream_get_line@test");
  fseek($f, 0);
  VS(stream_get_line($f, 300, "@"), "stream_get_line");
  VS(stream_get_line($f, 300, "@"), "test");
  VS(stream_get_line($f, 300, "@"), false);
  fclose($f);
}

function test_stream_get_meta_data() {
  list($port, $address, $server) = retry_bind_server();

  $errno = null;
  $errstr = null;
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  stream_set_timeout($client, 0, 500 * 1000); // 500ms
  $line = fgets($client);
  $meta = stream_get_meta_data($client);
  VS($meta['timed_out'], true);
  VS($meta['blocked'], true);
}

function test_stream_misc() {
  $transports = stream_get_transports();
  VERIFY(count($transports) > 0);
  VS(in_array("ssl", $transports), true);
  VS(in_array("tls", $transports), true);

  $w = stream_get_wrappers();
  VS(in_array("file", $w), true);
  VS(in_array("http", $w), true);
}

function test_stream_select() {
  $f = fopen(__DIR__."/../ext_file/test_ext_file.txt", "r");
  $reads = array($f);
  $ignore = null;
  VERIFY(stream_select(
           inout $reads,
           inout $ignore,
           inout $ignore,
           0,
           0
         ) != false);
}

function test_stream_socket_recvfrom_tcp() {
  list($port, $address, $server) = retry_bind_server();
  $errno = null;
  $errstr = null;
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  $peername = null;
  $s = stream_socket_accept($server, -1.0, inout $peername);
  $text = "testing";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $buffer = stream_socket_recvfrom($s, 100, 0, inout $peername);
  VS($buffer, "testing");
}

function test_stream_socket_recvfrom_tcp6() {
  list($port, $address, $server) = retry_bind_server6();
  $errno = null;
  $errstr = null;
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  $peername = null;
  $s = stream_socket_accept($server, -1.0, inout $peername);
  $text = "testing6";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $buffer = stream_socket_recvfrom($s, 100, 0, inout $peername);
  VS($buffer, "testing6");
}

function test_stream_socket_recvfrom_udp() {
  list($port, $address, $server) = retry_bind_server(true);
  $errno = null;
  $errstr = null;
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  $text = "testing-udp";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $peername = null;
  $buffer = stream_socket_recvfrom($server, 100, 0, inout $peername);
  VS($buffer, "testing-udp");
}

function test_stream_socket_recvfrom_udp6() {
  list($port, $address, $server) = retry_bind_server(true);
  $errno = null;
  $errstr = null;
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  $text = "testing-udp6";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $peername = null;
  $buffer = stream_socket_recvfrom($server, 100, 0, inout $peername);
  VS($buffer, "testing-udp6");
}


function test_stream_socket_recvfrom_unix() {
  $tmpsock = tempnam('/tmp', 'vmstreamtest');
  unlink($tmpsock);

  $address = "unix://$tmpsock";
  $errno = null;
  $errstr = null;
  $server = stream_socket_server($address, inout $errno, inout $errstr);
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  $peername = null;
  $s = stream_socket_accept($server, -1.0, inout $peername);
  $text = "testing";
  VERIFY(socket_send($client, $text, 7, 0) != false);

  $buffer = stream_socket_recvfrom($s, 100, 0, inout $peername);
  VS($buffer, $text);
}

function test_stream_socket_sendto_issue324() {
  list($port, $address, $server) = retry_bind_server();
  $errno = null;
  $errstr = null;
  $client = stream_socket_client($address, inout $errno, inout $errstr);

  $peername = null;
  $s = stream_socket_accept($server, -1.0, inout $peername);
  $text = "testing";
  VERIFY(stream_socket_sendto($client, $text, 0, ''));

  $buffer = stream_socket_recvfrom($s, 100, 0, inout $peername);
  VS($buffer, $text);
}

function test_stream_socket_kind() {
  list($port, $address, $server) = retry_bind_server();
  var_dump($server);
}

function test_stream_socket_shutdown() {
  list($port, $address, $server) = retry_bind_server();
  VERIFY(stream_socket_shutdown($server, 0));
}

// Verify that the stream constants have been registered correctly
// by checking some of them
function test_stream_constants() {
  VS(STREAM_CLIENT_CONNECT, 4);
  VS(STREAM_SERVER_LISTEN, 8);
  VS(STREAM_IPPROTO_RAW, 255);
  VS(STREAM_SOCK_SEQPACKET, 5);
}


<<__EntryPoint>>
function main_ext_stream() {
test_stream_copy_to_stream();
test_stream_get_contents();
test_stream_get_line();
test_stream_get_meta_data();
test_stream_misc();
test_stream_select();
test_stream_socket_recvfrom_tcp();
test_stream_socket_recvfrom_tcp6();
test_stream_socket_recvfrom_udp();
test_stream_socket_recvfrom_udp6();
test_stream_socket_recvfrom_unix();
test_stream_socket_sendto_issue324();
test_stream_socket_shutdown();
test_stream_socket_kind();
test_stream_constants();
}
