<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

// so we run on different range of ports every time
function get_random_port() {
  static $base = -1;
  if ($base == -1) {
    $base = 12345 + (int)((int)(microtime(false) * 100) % 30000);
  }
  return ++$base;
}

// On the continuous integration server, it's not unlikely that we'll
// fail to bind one of these random ports.  Retry a few times and hope
// for the best.
function retry_bind_server($udp = false) {
  for ($i = 0; $i < 20; ++$i) {
    $port = get_random_port();
    $scheme = $udp ? "udp://" : "tcp://";
    $address = $scheme."127.0.0.1:" . $port;

    if ($udp) {
      $server = @stream_socket_server($address, $errno, $errstr,
                                     STREAM_SERVER_BIND);
    } else {
      $server = @stream_socket_server($address);
    }
    if ($server !== false) {
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

    if ($udp) {
      $server = @stream_socket_server($address, $errno, $errstr,
                                     STREAM_SERVER_BIND);
    } else {
      $server = @stream_socket_server($address);
    }
    if ($server !== false) {
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
  VS(stream_get_line($f, 300, "@"), "");
  fclose($f);
}

function test_stream_get_meta_data() {
  list($port, $address, $server) = retry_bind_server();

  $client = stream_socket_client($address);

  stream_set_timeout($client, 0, 500 * 1000); // 500ms
  $line = fgets($client);
  $meta = stream_get_meta_data($client);
  VS($meta['timed_out'], true);
  VS($meta['blocked'], false);
}

function test_stream_misc() {
  VERIFY(count(stream_get_transports()) > 0);

  $w = stream_get_wrappers();
  VS(in_array("file", $w), true);
  VS(in_array("http", $w), true);
}

function test_stream_wrapper_restore() {
  $count = count(stream_get_wrappers());

  VS(stream_wrapper_unregister("http"), true);
  VS(count(stream_get_wrappers()), $count - 1);

  VS(stream_wrapper_restore("http"), true);
  VS(count(stream_get_wrappers()), $count);
}

function test_stream_select() {
  $f = fopen(__DIR__."/../ext_file/test_ext_file.txt", "r");
  $reads = array($f);
  VERIFY(stream_select($reads, $ignore, $ignore, 0, 0) != false);
}

function test_stream_socket_recvfrom_tcp() {
  list($port, $address, $server) = retry_bind_server();
  $client = stream_socket_client($address);

  $s = stream_socket_accept($server);
  $text = "testing";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $buffer = stream_socket_recvfrom($s, 100);
  VS($buffer, "testing");
}

function test_stream_socket_recvfrom_tcp6() {
  list($port, $address, $server) = retry_bind_server6();
  $client = stream_socket_client($address);

  $s = stream_socket_accept($server);
  $text = "testing6";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $buffer = stream_socket_recvfrom($s, 100);
  VS($buffer, "testing6");
}

function test_stream_socket_recvfrom_udp() {
  list($port, $address, $server) = retry_bind_server(true);
  $client = stream_socket_client($address);

  $text = "testing-udp";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $buffer = stream_socket_recvfrom($server, 100);
  VS($buffer, "testing-udp");
}

function test_stream_socket_recvfrom_udp6() {
  list($port, $address, $server) = retry_bind_server(true);
  $client = stream_socket_client($address);

  $text = "testing-udp6";
  VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);

  $buffer = stream_socket_recvfrom($server, 100);
  VS($buffer, "testing-udp6");
}


function test_stream_socket_recvfrom_unix() {
  $tmpsock = tempnam('/tmp', 'vmstreamtest');
  unlink($tmpsock);

  $address = "unix://$tmpsock";
  $server = stream_socket_server($address);
  $client = stream_socket_client($address);

  $s = stream_socket_accept($server);
  $text = "testing";
  VERIFY(socket_send($client, $text, 7, 0) != false);

  $buffer = stream_socket_recvfrom($s, 100);
  VS($buffer, $text);
}

function test_stream_socket_sendto_issue324() {
  list($port, $address, $server) = retry_bind_server();
  $client = stream_socket_client($address);

  $s = stream_socket_accept($server);
  $text = "testing";
  VERIFY(stream_socket_sendto($client, $text, 0, ''));

  $buffer = stream_socket_recvfrom($s, 100);
  VS($buffer, $text);
}

function test_stream_socket_shutdown() {
  list($port, $address, $server) = retry_bind_server();
  VERIFY(stream_socket_shutdown($server, 0));
}

test_stream_copy_to_stream();
test_stream_get_contents();
test_stream_get_line();
test_stream_get_meta_data();
test_stream_misc();
test_stream_wrapper_restore();
test_stream_select();
test_stream_socket_recvfrom_tcp();
test_stream_socket_recvfrom_tcp6();
test_stream_socket_recvfrom_udp();
test_stream_socket_recvfrom_udp6();
test_stream_socket_recvfrom_unix();
test_stream_socket_sendto_issue324();
test_stream_socket_shutdown();
