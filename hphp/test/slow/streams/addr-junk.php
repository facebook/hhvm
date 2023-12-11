<?hh

abstract final class GetRandomPortStatics {
  public static $base = -1;
}

// borrowed from test/slow/ext_stream/ext_stream.php
function get_random_port() :mixed{
  if (GetRandomPortStatics::$base == -1) {
    GetRandomPortStatics::$base = 12345 + (int)((int)(HH\Lib\Legacy_FIXME\cast_for_arithmetic(microtime(false)) * 100) % 30000);
  }
  return ++GetRandomPortStatics::$base;
}

function retry_bind_server() :mixed{
  for ($i = 0; $i < 20; ++$i) {
    $port = get_random_port();
    $address = "tcp://127.0.0.1:" . $port;

    $errno = null;
    $errstr = null;
    $server = @stream_socket_server($address, inout $errno, inout $errstr);
    if ($server !== false) {
      return vec[$port, $address, $server];
    }
  }
  throw new Exception("Couldn't bind server");
}


<<__EntryPoint>>
function main_addr_junk() :mixed{
list($port, $addr, $server) = retry_bind_server();
$client_addr = $addr . "/foo/bar/baz";
$errno = null;
$errstr = null;
$client = stream_socket_client($client_addr, inout $errno, inout $errstr);
$peername = null;
$s = stream_socket_accept($server, -1.0, inout $peername);
var_dump($client);
}
