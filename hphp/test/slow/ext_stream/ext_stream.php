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
  $port = get_random_port();
  $address = "127.0.0.1:" . $port;

  $server = stream_socket_server($address);
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

function test_stream_socket_recvfrom() {
  $port = get_random_port();
  $addresses = array(
    "127.0.0.1:" . $port, // TCP
    "unix:///tmp/test_stream.sock" // UNIX domain
  );
  unlink("/tmp/test_stream.sock");

  $i = 0;
  foreach ($addresses as $address) {
    $server = stream_socket_server($address);
    $client = stream_socket_client($address);

    $s = stream_socket_accept($server);
    $text = "testing";
    if ($i++ == 1) {
      VERIFY(socket_send($client, $text, 7, 0) != false);
    } else {
      VERIFY(stream_socket_sendto($client, $text, 0, $address) != false);
    }

    $buffer = stream_socket_recvfrom($s, 100);
    VS($buffer, "testing");
  }
}

function test_stream_socket_sendto_issue324() {
  $port = get_random_port();
  $address = "127.0.0.1:" . $port;

  $server = stream_socket_server($address);
  $client = stream_socket_client($address);

  $s = stream_socket_accept($server);
  $text = "testing";
  VERIFY(stream_socket_sendto($client, $text, 0, ''));

  $buffer = stream_socket_recvfrom($s, 100);
  VS($buffer, "testing");
}

function test_stream_socket_shutdown() {
  $port = get_random_port();
  $address = "127.0.0.1:" . $port;
  $server = stream_socket_server($address);
  VERIFY(stream_socket_shutdown($server, 0));
}

test_stream_copy_to_stream();
test_stream_get_contents();
test_stream_get_line();
test_stream_get_meta_data();
test_stream_misc();
test_stream_wrapper_restore();
test_stream_select();
test_stream_socket_recvfrom();
test_stream_socket_sendto_issue324();
test_stream_socket_shutdown();
